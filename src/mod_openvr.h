#ifndef MOD_OPENVR_H_
#define MOD_OPENVR_H_

#include <gmath/gmath.h>
#include <openvr.h>
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
	vr::TrackedDevicePose_t vr_pose[vr::k_unMaxTrackedDeviceCount];
	Mat4 xform[vr::k_unMaxTrackedDeviceCount];

	Mat4 eye_to_hmd_xform[2];
	Mat4 eye_xform[2], eye_inv_xform[2];

	int win_width, win_height;	// for the mirror texture

public:
	ModuleOpenVR();
	~ModuleOpenVR();

	bool init();
	void destroy();

	ModuleType get_type() const;
	const char *get_name() const;

	bool detect();

	void start();
	void stop();

	void set_origin_mode(goatvr_origin_mode mode);
	void recenter();

	bool have_headtracking() const;

	void update();

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_done();
	void draw_mirror();

	Mat4 get_view_matrix(int eye);
	Mat4 get_proj_matrix(int eye, float znear, float zfar);
};

}	// namespace goatvr

#endif	// MOD_OPENVR_H_
