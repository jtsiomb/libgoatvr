#ifndef MOD_OCULUS_H_
#define MOD_OCULUS_H_

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include "module.h"

namespace goatvr {

class ModuleOculus : public Module {
protected:
	RenderTexture rtex;
	ovrSession ovr;
	ovrHmdDesc hmd;
	ovrEyeRenderDesc rdesc[2];
	ovrGraphicsLuid ovr_luid;
	ovrTextureSwapChainData *ovr_rtex;
	ovrLayerEyeFov ovr_layer;

public:
	ModuleOculus();
	~ModuleOculus();

	bool init();
	void destroy();

	ModuleType get_type() const;
	const char *get_name() const;

	void detect();

	void start();
	void stop();

	void update();

	void set_fbsize(int width, int height, float fbscale);
	RenderTexture *get_render_texture();

	void draw_done();

	Mat4 get_view_matrix(int eye);
	Mat4 get_proj_matrix(int eye, float znear, float zfar);
};

}	// namespace goatvr

#endif	// MOD_OCULUS_H_
