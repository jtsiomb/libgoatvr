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

	bool have_headtracking() const;

	void update();

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_start();
	void draw_done();
	void draw_mirror();

	Mat4 get_view_matrix(int eye);
	Mat4 get_proj_matrix(int eye, float znear, float zfar);
};

}	// namespace goatvr

#endif	// MOD_OCULUS_H_
