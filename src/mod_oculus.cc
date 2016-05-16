#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_oculus.h"
#include "goatvr_impl.h"

using namespace goatvr;

ModuleOculus::ModuleOculus()
{
	memset(&rtex, 0, sizeof rtex);
	rtex.fbscale = 1.0f;
	ovr_rtex = 0;
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

void ModuleOculus::detect()
{
	print_info("running HMD detection\n");

	ovrHmdDesc hmd = ovr_GetHmdDesc(0);
	if(hmd.Type == ovrHmd_None) {
		print_info("none available\n");
		avail = false;
		return;
	}

	print_info("HMD found: %s - %s\n", hmd.Manufacturer, hmd.ProductName);
	avail = true;
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

	// force creating the render target when start is called
	get_render_texture();
}

void ModuleOculus::stop()
{
	if(!ovr) return;	// not started

	if(ovr_rtex) {
		ovr_DestroyTextureSwapChain(ovr, ovr_rtex);
		ovr_rtex = 0;
	}
	ovr_Destroy(ovr);
	ovr = 0;
}

void ModuleOculus::update()
{
	ovrVector3f eye_offs[2] = { rdesc[0].HmdToEyeOffset, rdesc[1].HmdToEyeOffset };

	double tm = ovr_GetPredictedDisplayTime(ovr, 0);
	ovrTrackingState tstate = ovr_GetTrackingState(ovr, tm, ovrTrue);
	ovr_CalcEyePoses(tstate.HeadPose.ThePose, eye_offs, ovr_layer.RenderPose);
}

void ModuleOculus::set_fbsize(int width, int height, float fbscale)
{
	rtex.fbscale = fbscale;
}

RenderTexture *ModuleOculus::get_render_texture()
{
	ovrSizei texsz[2];
	for(int i=0; i<2; i++) {
		ovrEyeType eye = (ovrEyeType)i;
		texsz[0] = ovr_GetFovTextureSize(ovr, eye, hmd.DefaultEyeFov[i], rtex.fbscale);
		rdesc[2] = ovr_GetRenderDesc(ovr, eye, hmd.DefaultEyeFov[i]);

		rtex.eye_width[i] = texsz[i].w;
		rtex.eye_height[i] = texsz[i].h;

		rtex.eye_yoffs[i] = 0;
	}
	rtex.eye_xoffs[0] = 0;
	rtex.eye_xoffs[1] = rtex.eye_xoffs[0] + rtex.eye_width[0];

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
	// TODO: this is certainly incorrect, cross-check with old code
	ovr_layer.Header.Type = ovrLayerType_EyeFov;
	ovr_layer.Header.Flags = 0;
	for(int i=0; i<2; i++) {
		ovr_layer.ColorTexture[i] = ovr_rtex;
		ovr_layer.Fov[i] = rdesc[i].Fov;
		ovr_layer.Viewport[i].Pos = {rtex.eye_xoffs[i], rtex.eye_yoffs[i]};
		ovr_layer.Viewport[i].Size = {rtex.eye_width[i], rtex.eye_height[i]};
	}
	return 0;
}

void ModuleOculus::draw_done()
{
	ovr_CommitTextureSwapChain(ovr, ovr_rtex);

	ovrLayerHeader *layers = &ovr_layer.Header;
	ovrResult res = ovr_SubmitFrame(ovr, 0, 0, &layers, 1);
	switch(res) {
	case ovrSuccess_NotVisible:
		print_info("lost HMD ownership\n");
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

Mat4 ModuleOculus::get_view_matrix(int eye)
{
	return Mat4::identity;
}

Mat4 ModuleOculus::get_proj_matrix(int eye, float znear, float zfar)
{
	Mat4 m = *(Mat4*)&ovrMatrix4f_Projection(rdesc[eye].Fov, znear, zfar, ovrProjection_ClipRangeOpenGL);
	return transpose(m);
}