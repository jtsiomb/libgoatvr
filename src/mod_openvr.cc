/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2017  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef USE_MOD_OPENVR

#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_openvr.h"
#include "goatvr_impl.h"

REG_MODULE(openvr, ModuleOpenVR)

using namespace goatvr;
using namespace vr;		// OpenVR namespace

static void openvr_matrix4(Mat4 &res, const HmdMatrix44_t &mat);
static void openvr_matrix(Mat4 &res, const HmdMatrix34_t &mat);
static VRTextureBounds_t openvr_tex_bounds(float umin, float vmin, float umax, float vmax);

ModuleOpenVR::ModuleOpenVR()
{
	vr = 0;
	vrcomp = 0;
	win_width = win_height = -1;
	rtex_valid = false;

	memset(xform_valid, 0, sizeof xform_valid);
}

ModuleOpenVR::~ModuleOpenVR()
{
	destroy();
}

bool ModuleOpenVR::init()
{
	if(!Module::init()) {
		return false;
	}
	return true;
}

void ModuleOpenVR::destroy()
{
	stop();
	Module::destroy();
}

enum goatvr_module_type ModuleOpenVR::get_type() const
{
	return GOATVR_DISPLAY_MODULE;
}

const char *ModuleOpenVR::get_name() const
{
	return "openvr";
}

bool ModuleOpenVR::detect()
{
	print_info("running HMD detection\n");

	if(!VR_IsHmdPresent()) {
		print_info("none available\n");
		avail = false;
		return false;
	}

	print_info("HMD found\n");
	avail = true;
	return true;
}

bool ModuleOpenVR::start()
{
	if(vr) return true;	// already started

	EVRInitError vrerr;
	if(!(vr = VR_Init(&vrerr, VRApplication_Scene))) {
		print_error("failed to initialize OpenVR: %s\n",
			VR_GetVRInitErrorAsEnglishDescription(vrerr));
		return false;
	}
	if(!(vrcomp = VRCompositor())) {
		print_error("failed to initialize OpenVR compositor\n");
		VR_Shutdown();
		vr = 0;
		return false;
	}
	if(!(vrchap = VRChaperone())) {
		print_error("failed to initialize OpenVR chaperone\n");
		VR_Shutdown();
		vr = 0;
		return false;
	}

	// grab the eye to HMD matrices
	for(int i=0; i<2; i++) {
		EVREye eye = i == 0 ? Eye_Left : Eye_Right;
		openvr_matrix(eye_to_hmd_xform[i], vr->GetEyeToHeadTransform(eye));
	}

	TrackedDeviceIndex_t idx;
	idx = vr->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
	hand_idx[0] = idx == k_unTrackedDeviceIndexInvalid ? -1 : (int)idx;
	idx = vr->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand);
	hand_idx[1] = idx == k_unTrackedDeviceIndexInvalid ? -1 : (int)idx;

	uint32_t x, y;
	vr->GetRecommendedRenderTargetSize(&x, &y);
	def_fbwidth = x;
	def_fbheight = y;

	// force creation of the render target when start is called
	get_render_texture();
	return true;
}

void ModuleOpenVR::stop()
{
	if(!vr) return;	// not started

	// if we call VR_Shutdown while a frame is pending, we'll crash
	vrcomp->ClearLastSubmittedFrame();
	VR_Shutdown();
	vr = 0;

	if(rtex.tex) {
		glDeleteTextures(1, &rtex.tex);
		rtex.tex = 0;
	}
	rtex_valid = false;
}

void ModuleOpenVR::update()
{
	// XXX is this going to block?
	vrcomp->WaitGetPoses(vr_pose, k_unMaxTrackedDeviceCount, 0, 0);

	// process OpenVR events (TODO)
	VREvent_t ev;
	while(vr->PollNextEvent(&ev, sizeof ev)) {
		switch(ev.eventType) {
		case VREvent_TrackedDeviceActivated:
			print_info("Device %u activated\n", ev.trackedDeviceIndex);
			break;

		case VREvent_TrackedDeviceDeactivated:
			print_info("Device %u deactivated\n", ev.trackedDeviceIndex);
			break;

		case VREvent_TrackedDeviceUpdated:
			print_info("Device %u updated\n", ev.trackedDeviceIndex);
			break;

		default:
			break;
		}
	}

	for(int i=0; i<(int)k_unMaxTrackedDeviceCount; i++) {
		if((xform_valid[i] = vr_pose[i].bPoseIsValid)) {
			openvr_matrix(xform[i], vr_pose[i].mDeviceToAbsoluteTracking);
		}

		// TODO buttons and stuff?
		/*
		VRControllerState_t st;
		if(vr->GetControllerState(i, &st)) {
			// ... stuff ..
		}
		*/
	}

	const Mat4 &hmd_xform = xform[k_unTrackedDeviceIndex_Hmd];
	for(int i=0; i<2; i++) {
		eye_xform[i] = eye_to_hmd_xform[i] * hmd_xform;
		eye_inv_xform[i] = inverse(eye_xform[i]);
	}
}

void ModuleOpenVR::set_origin_mode(goatvr_origin_mode mode)
{
	if(!vr) return;

	if(mode == GOATVR_FLOOR) {
		vrcomp->SetTrackingSpace(TrackingUniverseStanding);
	} else {
		vrcomp->SetTrackingSpace(TrackingUniverseSeated);
	}
}

void ModuleOpenVR::recenter()
{
#ifdef MOD_OPENVR_OLDRECENTER
	vr->ResetSeatedZeroPose();
#else
	vrchap->ResetZeroPose(TrackingUniverseSeated);
#endif
}

bool ModuleOpenVR::have_headtracking() const
{
	return true;
}

bool ModuleOpenVR::have_handtracking() const
{
	return hand_idx[0] >= 0 || hand_idx[1] >= 0;
}

bool ModuleOpenVR::hand_active(int idx) const
{
	return hand_idx[idx] >= 0 && xform_valid[hand_idx[idx]];
}

void ModuleOpenVR::set_fbsize(int width, int height, float fbscale)
{
	if(fbscale != rtex.fbscale) {
		rtex_valid = false;
	}
	rtex.fbscale = fbscale;
	// this is only used for the mirror texture
	win_width = width;
	win_height = height;
}

RenderTexture *ModuleOpenVR::get_render_texture()
{
	if(!rtex_valid) {
		for(int i=0; i<2; i++) {
			rtex.eye_width[i] = (int)((float)def_fbwidth * rtex.fbscale);
			rtex.eye_height[i] = (int)((float)def_fbheight * rtex.fbscale);
			rtex.eye_yoffs[i] = 0;
		}
		rtex.eye_xoffs[0] = 0;
		rtex.eye_xoffs[1] = rtex.eye_width[0];

		int fbwidth = rtex.eye_width[0] + rtex.eye_width[1];
		int fbheight = std::max(rtex.eye_height[0], rtex.eye_height[1]);

		rtex.update(fbwidth, fbheight);

		// prepare the OpenVR texture and texture bounds structs
		vr_tex.handle = (void*)rtex.tex;
		vr_tex.eType = TextureType_OpenGL;
		vr_tex.eColorSpace = ColorSpace_Linear;

		float umax = (float)rtex.width / (float)rtex.tex_width;
		float vmax = (float)rtex.height / (float)rtex.tex_height;
		vr_tex_bounds[0] = openvr_tex_bounds(0, 1.0 - vmax, 0.5 * umax, 1.0);
		vr_tex_bounds[1] = openvr_tex_bounds(0.5 * umax, 1.0 - vmax, umax, 1.0);

		// make sure we have the correct viewport in case the user never called goatvr_set_fb_size
		if(win_width == -1) {
			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			win_width = vp[2] + vp[0];
			win_height = vp[3] + vp[1];
		}

		rtex_valid = true;
	}
	return &rtex;
}

void ModuleOpenVR::draw_done()
{
	vrcomp->Submit(Eye_Left, &vr_tex, vr_tex_bounds);
	vrcomp->Submit(Eye_Right, &vr_tex, vr_tex_bounds + 1);

	glFlush();

	/* this is supposed to tell the compositor to get on with showing the frame without waiting for
	 * the next WaitGetPoses call.
	 */
	vrcomp->PostPresentHandoff();
}

void ModuleOpenVR::draw_mirror()
{
	return;	// XXX it looks like maybe the mirror window drawing fucks this up ... no idea why
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, win_width, win_height);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rtex.tex);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor3f(1, 1, 1);
	float umax = (float)rtex.width / (float)rtex.tex_width;
	float vmax = (float)rtex.height / (float)rtex.tex_height;
	glTexCoord2f(0, 0); glVertex2f(-1, -1);
	glTexCoord2f(umax, 0); glVertex2f(1, -1);
	glTexCoord2f(umax, vmax); glVertex2f(1, 1);
	glTexCoord2f(0, vmax); glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void ModuleOpenVR::get_view_matrix(Mat4 &mat, int eye) const
{
	mat = eye_inv_xform[eye];
}

void ModuleOpenVR::get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const
{
	EVREye openvr_eye = eye == GOATVR_LEFT ? Eye_Left : Eye_Right;
	openvr_matrix4(mat, vr->GetProjectionMatrix(openvr_eye, znear, zfar));
}

Vec3 ModuleOpenVR::get_head_position() const
{
	return Module::get_head_position();	// TODO
}

Quat ModuleOpenVR::get_head_orientation() const
{
	return Module::get_head_orientation();	// TODO
}

void ModuleOpenVR::get_head_matrix(Mat4 &mat) const
{
	mat = xform[k_unTrackedDeviceIndex_Hmd];
}

Vec3 ModuleOpenVR::get_hand_position(int idx) const
{
	return Module::get_hand_position(idx);	// TODO
}

Quat ModuleOpenVR::get_hand_orientation(int idx) const
{
	return Module::get_hand_orientation(idx);	// TODO
}

void ModuleOpenVR::get_hand_matrix(Mat4 &mat, int idx) const
{
	if(hand_idx[idx] >= 0) {
		mat = xform[hand_idx[idx]];
	}
}


static void openvr_matrix4(Mat4 &res, const HmdMatrix44_t &mat)
{
	res = Mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
}

static void openvr_matrix(Mat4 &res, const HmdMatrix34_t &mat)
{
	res = Mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0,
			mat.m[0][1], mat.m[1][1], mat.m[2][1], 0,
			mat.m[0][2], mat.m[1][2], mat.m[2][2], 0,
			mat.m[0][3], mat.m[1][3], mat.m[2][3], 1);
}

static VRTextureBounds_t openvr_tex_bounds(float umin, float vmin, float umax, float vmax)
{
	VRTextureBounds_t res;
	res.uMin = umin;
	res.uMax = umax;
	res.vMin = vmin;
	res.vMax = vmax;
	return res;
}

#else
#include "module.h"
// this expands to an empty register_mod_openvr() function
NOREG_MODULE(openvr)
#endif	// USE_MOD_OPENVR
