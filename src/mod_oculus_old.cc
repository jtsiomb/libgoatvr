// TODO make a VR compositor thread with its own GL context and window

#ifdef USE_MOD_OCULUS_OLD

#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_oculus_old.h"
#include "goatvr_impl.h"

REG_MODULE(oculus_old, ModuleOculusOld)

using namespace goatvr;

ModuleOculusOld::ModuleOculusOld()
{
	hmd = 0;

	memset(&rtex, 0, sizeof rtex);
	rtex.fbscale = 1.0f;
}

ModuleOculusOld::~ModuleOculusOld()
{
	destroy();
}

bool ModuleOculusOld::init()
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

void ModuleOculusOld::destroy()
{
	if(hmd) {
		ovrHmd_Destroy(hmd);
	}
	ovr_Shutdown();
	Module::destroy();
}

ModuleType ModuleOculusOld::get_type() const
{
	return MODULE_RENDERING;
}

const char *ModuleOculusOld::get_name() const
{
	return "oculus";
}

bool ModuleOculusOld::detect()
{
	print_info("running HMD detection\n");

	if(ovrHmd_Detect() <= 0) {
		print_info("none available\n");
		avail = false;
		return false;
	}

	print_info("HMD found\n");
	avail = true;
	return true;
}

void ModuleOculusOld::start()
{
	if(hmd) return;		// already started

	if(!(hmd = ovrHmd_Create(0))) {
		print_error("failed to create oculus HMD\n");
		return;
	}

	if(win_width == -1) {
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		win_width = vp[2];
		win_height = vp[3];
	}

	memset(&ovr_glcfg, 0, sizeof ovr_glcfg);
	ovr_glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	ovr_glcfg.OGL.Header.BackBufferSize.w = win_width;
	ovr_glcfg.OGL.Header.BackBufferSize.h = win_height;
	ovr_glcfg.OGL.Header.Multisample = 1;

#ifdef OVR_OS_WIN32
	ovr_glcfg.OGL.Window = GetActiveWindow();
	ovr_glcfg.OGL.DC = wglGetCurrentDC();
#elif defined(OVR_OS_LINUX)
	ovr_glcfg.OGL.Disp = glXGetCurrentDisplay();
#endif

	unsigned int hmd_caps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);

	unsigned int distort_caps = ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive;
#ifdef OVR_OS_LINUX
	distort_caps |= ovrDistortionCap_LinuxDevFullscreen;
#endif
	ovrHmd_ConfigureRendering(hmd, &ovr_glcfg.Config, distort_caps, hmd->DefaultEyeFov, ovr_rdesc);

	// force creation of the render target when start is called
	get_render_texture();
}

void ModuleOculusOld::stop()
{
	if(!hmd) return;	// not started

	ovrHmd_Destroy(hmd);
	hmd = 0;
}

void ModuleOculusOld::set_origin_mode(goatvr_origin_mode mode)
{
	origin_mode = mode;
}

void ModuleOculusOld::recenter()
{
	if(hmd) {
		ovr_RecenterPose(hmd);
	}
}

void ModuleOculusOld::update()
{
	ovrVector3f eye_offs[2] = {
		ovr_rdesc[0].HmdToEyeViewOffset,
		ovr_rdesc[1].HmdToEyeViewOffset
	};

	ovrPosef poses[2];
	ovrTrackingState tstate;
	ovrHmd_GetEyePoses(hmd, 0, eye_offs, poses, &tstate);

	for(int i=0; i<2; i++) {
		ovrVector3f pos = poses[i].Position;
		ovrQuatf rot = poses[i].Orientation;

		eye_pos[i] = Vec3(pos.x, pos.y, pos.z);
		eye_rot[i] = Quat(rot.x, rot.y, rot.z, rot.w);

		Mat4 rmat = eye_rot[i].calc_matrix();
		Mat4 tmat;
		tmat.translation(eye_pos[i]);
		eye_xform[i] = rmat * tmat;

		rmat.transpose();
		tmat.translation(-eye_pos[i]);
		eye_inv_xform[i] = tmat * rmat;
	}
	// TODO do we have to translate by eye_offs?
	// TODO if origin is floor, also translate by -OVR_KEY_EYE_HEIGHT
}

void ModuleOculusOld::set_fbsize(int width, int height, float fbscale)
{
	rtex.fbscale = fbscale;
	// this is only used for the mirror texture
	win_width = width;
	win_height = height;
}

// TODO continue from here...

RenderTexture *ModuleOculusOld::get_render_texture()
{
	// TODO cache some of these values and update them only when necessary
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
	
	return &rtex;
}

void ModuleOculusOld::draw_done()
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

void ModuleOculusOld::draw_mirror()
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

Mat4 ModuleOculusOld::get_view_matrix(int eye)
{
	return eye_inv_xform[eye];
}

Mat4 ModuleOculusOld::get_proj_matrix(int eye, float znear, float zfar)
{
	Mat4 m = *(Mat4*)&ovrMatrix4f_Projection(rdesc[eye].Fov, znear, zfar, ovrProjection_ClipRangeOpenGL);
	return transpose(m);
}

#else

#include "module.h"
// this expands to an empty register_mod_oculus() function
NOREG_MODULE(oculus_old)

#endif	// USE_MOD_OCULUS_OLD
