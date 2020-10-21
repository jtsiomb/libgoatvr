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
#ifndef MOD_OPENVR_H_
#define MOD_OPENVR_H_

#include <gmath/gmath.h>
#include <openvr/openvr.h>
#include "module.h"
#include "rtex.h"

namespace goatvr {

class ModuleOpenVR : public Module {
protected:
	int def_fbwidth, def_fbheight;	// recommended by OpenVR fb size

	RenderTexture rtex;
	bool rtex_valid;
	vr::Texture_t vr_tex;
	vr::VRTextureBounds_t vr_tex_bounds[2];

	vr::IVRSystem *vr;
	vr::IVRCompositor *vrcomp;
	vr::IVRChaperone *vrchap;
	vr::TrackedDevicePose_t vr_pose[vr::k_unMaxTrackedDeviceCount];
	Mat4 xform[vr::k_unMaxTrackedDeviceCount];
	bool xform_valid[vr::k_unMaxTrackedDeviceCount];
	int hand_idx[2];

	Mat4 eye_to_hmd_xform[2];
	Mat4 eye_xform[2], eye_inv_xform[2];

	int win_width, win_height;	// for the mirror texture

public:
	ModuleOpenVR();
	~ModuleOpenVR();

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
	bool hand_active(int idx) const;

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_done();
	void draw_mirror();

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

#endif	// MOD_OPENVR_H_
