#ifdef USE_MOD_OPENVR

#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_openvr.h"
#include "goatvr_impl.h"

REG_MODULE(openvr, ModuleOpenVR)

using namespace goatvr;
using namespace vr;		// OpenVR namespace

static Mat4 openvr_matrix4(const HmdMatrix44_t &mat);
static Mat4 openvr_matrix(const HmdMatrix34_t &mat);
static VRTextureBounds_t openvr_tex_bounds(float umin, float vmin, float umax, float vmax);

ModuleOpenVR::ModuleOpenVR()
{
	memset(&rtex, 0, sizeof rtex);
	rtex.fbscale = 1.0f;

	vr = 0;
	vrcomp = 0;
	win_width = win_height = -1;
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

ModuleType ModuleOpenVR::get_type() const
{
	return MODULE_RENDERING;
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

void ModuleOpenVR::start()
{
	if(vr) return;	// already started

	EVRInitError vrerr;
	if(!(vr = VR_Init(&vrerr, VRApplication_Scene))) {
		print_error("failed to initialize OpenVR: %s\n",
			VR_GetVRInitErrorAsEnglishDescription(vrerr));
		return;
	}
	if(!(vrcomp = VRCompositor())) {
		print_error("failed to initialize OpenVR compositor\n");
		VR_Shutdown();
		vr = 0;
		return;
	}

	// grab the eye to HMD matrices
	for(int i=0; i<2; i++) {
		EVREye eye = i == 0 ? Eye_Left : Eye_Right;
		eye_to_hmd_xform[i] = openvr_matrix(vr->GetEyeToHeadTransform(eye));
	}

	uint32_t x, y;
	vr->GetRecommendedRenderTargetSize(&x, &y);
	def_fbwidth = x;
	def_fbheight = y;

	// force creation of the render target when start is called
	get_render_texture();
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
	vr->ResetSeatedZeroPose();
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

	for(int i=0; i<k_unMaxTrackedDeviceCount; i++) {
		if(vr_pose[i].bPoseIsValid) {
			xform[i] = openvr_matrix(vr_pose[i].mDeviceToAbsoluteTracking);
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

void ModuleOpenVR::set_fbsize(int width, int height, float fbscale)
{
	rtex.fbscale = fbscale;
	// this is only used for the mirror texture
	win_width = width;
	win_height = height;
}

RenderTexture *ModuleOpenVR::get_render_texture()
{
	for(int i=0; i<2; i++) {
		rtex.eye_width[i] = (int)((float)def_fbwidth * rtex.fbscale);
		rtex.eye_height[i] = (int)((float)def_fbheight * rtex.fbscale);
		rtex.eye_yoffs[i] = 0;
	}
	rtex.eye_xoffs[0] = 0;
	rtex.eye_xoffs[1] = rtex.eye_width[0];

	int fbwidth = rtex.eye_width[0] + rtex.eye_width[1];
	int fbheight = std::max(rtex.eye_height[0], rtex.eye_height[1]);

	int texwidth = next_pow2(fbwidth);
	int texheight = next_pow2(fbheight);

	if(!rtex.tex) {
		glGenTextures(1, &rtex.tex);
		glBindTexture(GL_TEXTURE_2D, rtex.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		rtex.tex_width = -1; // invalidate width to force a tex rebuild
	}

	// recreate the texture if necessary
	if(rtex.tex_width != texwidth || rtex.tex_height != texheight) {
		print_info("creating %dx%d texture for %dx%d framebuffer\n", texwidth, texheight, fbwidth, fbheight);
		glBindTexture(GL_TEXTURE_2D, rtex.tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		rtex.tex_width = texwidth;
		rtex.tex_height = texheight;
	}

	rtex.width = fbwidth;
	rtex.height = fbheight;

	// prepare the OpenVR texture and texture bounds structs
	vr_tex.handle = (void*)rtex.tex;
	vr_tex.eType = API_OpenGL;
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

Mat4 ModuleOpenVR::get_view_matrix(int eye)
{
	return eye_inv_xform[eye];
}

Mat4 ModuleOpenVR::get_proj_matrix(int eye, float znear, float zfar)
{
	EVREye openvr_eye = eye == GOATVR_LEFT ? Eye_Left : Eye_Right;
	return openvr_matrix4(vr->GetProjectionMatrix(openvr_eye, znear, zfar, API_OpenGL));
}


static Mat4 openvr_matrix4(const HmdMatrix44_t &mat)
{
	return Mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]);
}

static Mat4 openvr_matrix(const HmdMatrix34_t &mat)
{
	return Mat4(mat.m[0][0], mat.m[1][0], mat.m[2][0], 0,
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
