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

	float eye_height;

	int win_width, win_height;

public:
	ModuleOculusOld();
	~ModuleOculusOld();

	bool init();
	void destroy();

	ModuleType get_type() const;
	const char *get_name() const;

	bool detect();

	void start();
	void stop();

	void set_origin_mode(goatvr_origin_mode mode);
	void recenter();

	void update();

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_start();
	void draw_done();

	Mat4 get_view_matrix(int eye);
	Mat4 get_proj_matrix(int eye, float znear, float zfar);
};

}	// namespace goatvr

#endif	// MOD_OCULUS_OLD_H_
