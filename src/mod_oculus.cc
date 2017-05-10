/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2016  John Tsiombikas <nuclear@member.fsf.org>

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
#ifdef USE_MOD_OCULUS

#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_oculus.h"
#include "goatvr_impl.h"

struct SrcData {
	char name[32];
	bool valid;
	Vec3 pos;
	Quat rot;
};

REG_MODULE(oculus, ModuleOculus)

using namespace goatvr;

ModuleOculus::ModuleOculus()
{
	ovr = 0;
	ovr_rtex = 0;

	ovr_mirtex = 0;
	mirtex_width = mirtex_height = -1;
	win_width = win_height = -1;

	rtex_valid = false;
}

ModuleOculus::~ModuleOculus()
{
	destroy();
}

bool ModuleOculus::init()
{
	if(!Module::init()) {
		return false;
	}

	if(ovr_Initialize(0) != 0) {
		print_error("failed to initialize libovr\n");
		return false;
	}
	return true;
}

void ModuleOculus::destroy()
{
	ovr_Shutdown();
	Module::destroy();
}

ModuleType ModuleOculus::get_type() const
{
	return MODULE_RENDERING;
}

const char *ModuleOculus::get_name() const
{
	return "oculus";
}

bool ModuleOculus::detect()
{
	print_info("running HMD detection\n");

	ovrHmdDesc hmd = ovr_GetHmdDesc(0);
	if(hmd.Type == ovrHmd_None) {
		print_info("none available\n");
		avail = false;
		return false;
	}

	print_info("HMD found: %s - %s\n", hmd.Manufacturer, hmd.ProductName);
	avail = true;
	return true;
}

void ModuleOculus::start()
{
	if(ovr) return;		// already started

	if(ovr_Create(&ovr, &ovr_luid) != 0) {
		print_error("failed to create oculus session\n");
		return;
	}

	hmd = ovr_GetHmdDesc(ovr);
	if(hmd.Type == ovrHmd_None) {
		print_error("failed to start, no HMD available!\n");
		ovr_Destroy(ovr);
		ovr = 0;
		return;
	}

	// force creation of the render target when start is called
	get_render_texture();

	// At least the head-tracking source is always available
	Source *src = new Source;
	SrcData *srcdata = new SrcData;
	src->mod = this;
	src->mod_data = srcdata;
	strcpy(srcdata->name, "oculus-head");

	inp_sources.push_back(src);
	def_track_src[0] = track_src[0] = src;
	add_source(src);

	unsigned int ctlmask = ovr_GetConnectedControllerTypes(ovr);
	printf("ctlmask: %u\n", ctlmask);

	if(ctlmask & ovrControllerType_Touch) {
		for(int i=0; i<2; i++) {
			src = new Source;
			srcdata = new SrcData;
			src->mod = this;
			src->mod_data = srcdata;
			strcpy(srcdata->name, i ? "oculus-rhand" : "oculus-lhand");
			srcdata->valid = false;

			inp_sources.push_back(src);
			def_track_src[i + 1] = track_src[i + 1] = src;
			add_source(src);
		}
	}
}

void ModuleOculus::stop()
{
	if(!ovr) return;	// not started

	for(int i=0; i<3; i++) {
		if(def_track_src[i]) {
			remove_source(def_track_src[i]);
			delete def_track_src[i];
			def_track_src[i] = track_src[i] = 0;
		}
	}
	inp_sources.clear();

	if(ovr_rtex) {
		ovr_DestroyTextureSwapChain(ovr, ovr_rtex);
		ovr_rtex = 0;
	}
	rtex_valid = false;
	ovr_Destroy(ovr);
	ovr = 0;
}

void ModuleOculus::set_origin_mode(goatvr_origin_mode mode)
{
	if(!ovr) return;	// not started

	if(mode == GOATVR_FLOOR) {
		ovr_SetTrackingOriginType(ovr, ovrTrackingOrigin_FloorLevel);
	} else {
		ovr_SetTrackingOriginType(ovr, ovrTrackingOrigin_EyeLevel);
	}
}

void ModuleOculus::recenter()
{
	if(ovr) {
		ovr_RecenterTrackingOrigin(ovr);
	}
}

const char *ModuleOculus::get_source_name(void *sdata) const
{
	SrcData *sd = (SrcData*)sdata;
	return sd->name;
}

bool ModuleOculus::is_source_spatial(void *sdata) const
{
	return true;
}

int ModuleOculus::get_source_num_axes(void *sdata) const
{
	return 0;	   // TODO
}

int ModuleOculus::get_source_num_buttons(void *sdata) const
{
	return 0;	   // TODO
}

Vec3 ModuleOculus::get_source_pos(void *sdata) const
{
	return ((SrcData*)sdata)->pos;
}

Quat ModuleOculus::get_source_rot(void *sdata) const
{
	return ((SrcData*)sdata)->rot;
}

static inline void update_source(Source *src, const ovrPosef &pose, float units_scale)
{
	SrcData *sd = (SrcData*)src->mod_data;

	ovrVector3f ovrpos = pose.Position;
	ovrQuatf ovrrot = pose.Orientation;

	sd->pos = Vec3(ovrpos.x, ovrpos.y, ovrpos.z) * units_scale;
	sd->rot = Quat(ovrrot.x, ovrrot.y, ovrrot.z, ovrrot.w);

	Mat4 rmat = sd->rot.calc_matrix();
	Mat4 tmat;
	tmat.translation(sd->pos);

	src->xform = rmat * tmat;
}

void ModuleOculus::update()
{
	float units_scale = goatvr_get_units_scale();
	ovrVector3f eye_offs[2] = { rdesc[0].HmdToEyeOffset, rdesc[1].HmdToEyeOffset };

	double tm = ovr_GetPredictedDisplayTime(ovr, 0);
	ovrTrackingState tstate = ovr_GetTrackingState(ovr, tm, ovrTrue);
	ovr_CalcEyePoses(tstate.HeadPose.ThePose, eye_offs, ovr_layer.RenderPose);

	// fill in the details for the head input source
	update_source(def_track_src[0], tstate.HeadPose.ThePose, units_scale);

	for(int i=0; i<2; i++) {
		ovrVector3f pos = ovr_layer.RenderPose[i].Position;
		ovrQuatf rot = ovr_layer.RenderPose[i].Orientation;

		eye_pos[i] = Vec3(pos.x, pos.y, pos.z) * units_scale;
		eye_rot[i] = Quat(rot.x, rot.y, rot.z, rot.w);

		Mat4 rmat = eye_rot[i].calc_matrix();
		Mat4 tmat;
		tmat.translation(eye_pos[i]);
		eye_xform[i] = rmat * tmat;

		rmat.transpose();
		tmat.translation(-eye_pos[i]);
		eye_inv_xform[i] = tmat * rmat;

		// also update hand tracking poses if available
		if(def_track_src[i + 1]) {
			SrcData *sd = (SrcData*)def_track_src[i + 1];

			sd->valid = (tstate.HandStatusFlags[i] & ovrStatus_PositionTracked) != 0;
			//if(sd->valid) {
				update_source(def_track_src[i + 1], tstate.HandPoses[i].ThePose, units_scale);
			//}
		}
	}
}

void ModuleOculus::set_fbsize(int width, int height, float fbscale)
{
	if(fbscale != rtex.fbscale) {
		rtex_valid = false;
	}
	rtex.fbscale = fbscale;
	// this is only used for the mirror texture
	win_width = width;
	win_height = height;
}

RenderTexture *ModuleOculus::get_render_texture()
{
	if(!rtex_valid) {
		ovrSizei texsz[2];
		for(int i=0; i<2; i++) {
			ovrEyeType eye = (ovrEyeType)i;
			texsz[i] = ovr_GetFovTextureSize(ovr, eye, hmd.DefaultEyeFov[i], rtex.fbscale);
			rdesc[i] = ovr_GetRenderDesc(ovr, eye, hmd.DefaultEyeFov[i]);

			rtex.eye_width[i] = texsz[i].w;
			rtex.eye_height[i] = texsz[i].h;
			rtex.eye_yoffs[i] = 0;
		}
		rtex.eye_xoffs[0] = 0;
		rtex.eye_xoffs[1] = rtex.eye_width[0];

		int fbwidth = texsz[0].w + texsz[1].w;
		int fbheight = std::max(texsz[0].h, texsz[1].h);

		int texwidth = next_pow2(fbwidth);
		int texheight = next_pow2(fbheight);

		// recreate the texture if necessary
		if(rtex.tex_width != texwidth || rtex.tex_height != texheight) {
			if(ovr_rtex) {	// we have a previous texture
				ovr_DestroyTextureSwapChain(ovr, ovr_rtex);
			}

			ovrTextureSwapChainDesc desc;
			memset(&desc, 0, sizeof desc);
			desc.Type = ovrTexture_2D;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.ArraySize = 1;	// ?
			desc.Width = texwidth;
			desc.Height = texheight;
			desc.MipLevels = 1;
			desc.SampleCount = 1;

			if(ovr_CreateTextureSwapChainGL(ovr, &desc, &ovr_rtex) != 0) {
				print_error("failed to create texture swap chain (%dx%d)\n", texwidth, texheight);
				ovr_rtex = 0;
				return 0;
			}

			rtex.tex_width = texwidth;
			rtex.tex_height = texheight;
		}

		rtex.width = fbwidth;
		rtex.height = fbheight;

		// NOTE: index -1 means current index
		ovr_GetTextureSwapChainBufferGL(ovr, ovr_rtex, -1, &rtex.tex);

		// prepare the layer
		ovr_layer.Header.Type = ovrLayerType_EyeFov;
		ovr_layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
		for(int i=0; i<2; i++) {
			ovr_layer.ColorTexture[i] = ovr_rtex;
			ovr_layer.Fov[i] = rdesc[i].Fov;
			//ovr_layer.Viewport[i].Pos = {rtex.eye_xoffs[i], rtex.eye_yoffs[i]};
			ovr_layer.Viewport[i].Pos = {rtex.eye_xoffs[i], texheight - fbheight};
			ovr_layer.Viewport[i].Size = {rtex.eye_width[i], rtex.eye_height[i]};
		}

		// create the mirror texture if necessary
		if(win_width == -1) {
			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			win_width = vp[2] + vp[0];
			win_height = vp[3] + vp[1];
		}

		int new_mtex_width = next_pow2(win_width);
		int new_mtex_height = next_pow2(win_height);
		if(!ovr_mirtex || mirtex_width != new_mtex_width || mirtex_height != new_mtex_height) {
			ovrMirrorTextureDesc desc;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.Width = new_mtex_width;
			desc.Height = new_mtex_height;

			if(ovr_CreateMirrorTextureGL(ovr, &desc, &ovr_mirtex) != 0) {
				print_error("failed to create mirror texture (%dx%d)\n", new_mtex_width, new_mtex_height);
				ovr_mirtex = 0;
			}
			mirtex_width = new_mtex_width;
			mirtex_height = new_mtex_height;

			ovr_GetMirrorTextureBufferGL(ovr, ovr_mirtex, &mirtex);

			glBindTexture(GL_TEXTURE_2D, mirtex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		rtex_valid = true;
	}
	return &rtex;
}

void ModuleOculus::draw_start()
{
	// NOTE: index -1 means current index
	ovr_GetTextureSwapChainBufferGL(ovr, ovr_rtex, -1, &rtex.tex);
}

void ModuleOculus::draw_done()
{
	ovr_CommitTextureSwapChain(ovr, ovr_rtex);

	ovrViewScaleDesc scale_desc;
	scale_desc.HmdSpaceToWorldScaleInMeters = 1.0 / goatvr_get_units_scale();
	scale_desc.HmdToEyeOffset[0] = rdesc[0].HmdToEyeOffset;
	scale_desc.HmdToEyeOffset[1] = rdesc[1].HmdToEyeOffset;
	ovrLayerHeader *layers = &ovr_layer.Header;
	ovrResult res = ovr_SubmitFrame(ovr, 0, &scale_desc, &layers, 1);
	switch(res) {
	case ovrSuccess_NotVisible:
		//print_info("lost HMD ownership\n");
		// TODO notify the app about this to throttle down
		break;

	case ovrError_DisplayLost:
		print_error("display lost\n");
		// TODO notify the app about this
		stop();
		break;

	case ovrError_TextureSwapChainInvalid:
		print_error("invalid swap chain!\n");
		stop();
		break;
	}
}

void ModuleOculus::draw_mirror()
{
	glViewport(0, 0, win_width, win_height);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mirtex);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

bool ModuleOculus::should_swap() const
{
	return true;
}

Mat4 ModuleOculus::get_view_matrix(int eye) const
{
	return eye_inv_xform[eye];
}

Mat4 ModuleOculus::get_proj_matrix(int eye, float znear, float zfar) const
{
	Mat4 m = *(Mat4*)&ovrMatrix4f_Projection(rdesc[eye].Fov, znear, zfar, ovrProjection_ClipRangeOpenGL);
	return transpose(m);
}

#else

#include "module.h"
// this expands to an empty register_mod_oculus() function
NOREG_MODULE(oculus)

#endif	// USE_MOD_OCULUS
