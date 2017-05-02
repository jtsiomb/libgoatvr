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
#ifndef MOD_OCULUS_H_
#define MOD_OCULUS_H_

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <gmath/gmath.h>
#include "module.h"

namespace goatvr {

class ModuleOculus : public Module {
protected:
	RenderTexture rtex;
	bool rtex_valid;
	ovrSession ovr;
	ovrHmdDesc hmd;
	ovrEyeRenderDesc rdesc[2];
	ovrGraphicsLuid ovr_luid;
	ovrTextureSwapChainData *ovr_rtex;
	ovrLayerEyeFov ovr_layer;

	double input_time;
	Vec3 eye_pos[2];
	Quat eye_rot[2];
	Mat4 eye_xform[2], eye_inv_xform[2];

	ovrMirrorTextureData *ovr_mirtex;
	unsigned int mirtex;
	int mirtex_width, mirtex_height;
	int win_width, win_height;


public:
	ModuleOculus();
	~ModuleOculus();

	bool init();
	void destroy();

	ModuleType get_type() const;
	const char *get_name() const;

	bool detect();

	void start();
	void stop();

	void set_origin_mode(goatvr_origin_mode mode);
	void recenter();

	const char *get_source_name(void *sdata) const;
	bool is_source_spatial(void *sdata) const;
	int get_source_num_axes(void *sdata) const;
	int get_source_num_buttons(void *sdata) const;
	Vec3 get_source_pos(void *sdata) const;
	Quat get_source_rot(void *sdata) const;

	void update();

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_start();
	void draw_done();
	void draw_mirror();

	bool should_swap() const;

	Mat4 get_view_matrix(int eye) const;
	Mat4 get_proj_matrix(int eye, float znear, float zfar) const;
};

}	// namespace goatvr

#endif	// MOD_OCULUS_H_
