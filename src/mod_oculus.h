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

	bool have_touch;

	PosRot head;
	PosRot hand[2];
	bool hand_valid[2];

	double input_time;
	PosRot eye[2];
	Mat4 eye_inv_xform[2];

	unsigned int bnstate, touchstate;
	Vec2 stick_pos[2];

	ovrMirrorTextureData *ovr_mirtex;
	unsigned int mirtex;
	int mirtex_width, mirtex_height;
	int win_width, win_height;


public:
	ModuleOculus();
	~ModuleOculus();

	bool init();
	void destroy();

	enum goatvr_module_type get_type() const;
	const char *get_name() const;

	bool detect();

	bool start();
	void stop();

	void update();

	void set_origin_mode(goatvr_origin_mode mode);
	void recenter();

	bool have_headtracking() const;
	bool have_handtracking() const;
	bool hand_active(int hand) const;

	int num_buttons() const;
	const char *get_button_name(int bn) const;
	unsigned int get_button_state(unsigned int mask) const;

	int num_axes() const;
	const char *get_axis_name(int axis) const;
	float get_axis_value(int axis) const;

	int num_sticks() const;
	const char *get_stick_name(int axis) const;
	Vec2 get_stick_pos(int stick) const;

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_start();
	void draw_done();
	void draw_mirror();

	bool should_swap() const;

	void get_view_matrix(Mat4 &mat, int eye) const;
	void get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const;

	Vec3 get_head_position() const;
	Quat get_head_orientation() const;
	void get_head_matrix(Mat4 &mat) const;

	Vec3 get_hand_position(int hand) const;
	Quat get_hand_orientation(int hand) const;
	void get_hand_matrix(Mat4 &mat, int hand) const;
};

}	// namespace goatvr

#endif	// MOD_OCULUS_H_
