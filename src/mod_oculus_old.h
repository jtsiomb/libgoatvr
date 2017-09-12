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
#ifndef MOD_OCULUS_OLD_H_
#define MOD_OCULUS_OLD_H_

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <gmath/gmath.h>
#include "module.h"

namespace goatvr {

class ModuleOculusOld : public Module {
protected:
	RenderTexture rtex;
	bool rtex_valid;
	ovrHmd hmd;
	ovrEyeRenderDesc ovr_rdesc[2];
	ovrGLTexture ovr_gltex[2];
	ovrGLConfig ovr_glcfg;
	ovrPosef ovr_poses[2];

	ovrHmdType fakehmd;

	goatvr_origin_mode origin_mode;

	Vec3 eye_pos[2];
	Quat eye_rot[2];
	Mat4 eye_xform[2], eye_inv_xform[2];

	Vec3 head_pos;
	Mat4 head_xform;

	float eye_height;

	int win_width, win_height;

public:
	ModuleOculusOld();
	~ModuleOculusOld();

	bool init();
	void destroy();

	enum goatvr_module_type get_type() const;
	const char *get_name() const;

	bool detect();

	void start();
	void stop();

	void update();

	void set_origin_mode(goatvr_origin_mode mode);
	void recenter();

	bool have_headtracking() const;

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_start();
	void draw_done();

	bool should_swap() const;

	void get_view_matrix(Mat4 &mat, int eye) const;
	void get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const;

	Vec3 get_head_position() const;
	Quat get_head_orientation() const;
	void get_head_matrix(Mat4 &mat) const;
};

}	// namespace goatvr

#endif	// MOD_OCULUS_OLD_H_
