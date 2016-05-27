// TODO make a VR compositor thread with its own GL context and window

#ifdef USE_MOD_OCULUS_OLD

#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "mod_oculus_old.h"
#include "goatvr_impl.h"
#include "wmutils.h"

#ifdef OVR_OS_LINUX
#include <GL/glx.h>
#endif
#ifdef OVR_OS_WIN32
#include <windows.h>
#endif

#ifdef _MSC_VER
#define strcasecmp	stricmp
#endif


REG_MODULE(oculus_old, ModuleOculusOld)

using namespace goatvr;

static Mat4 ovr_matrix(const ovrMatrix4f &mat);
static ovrHmdType parse_hmdtype(const char *s);



ModuleOculusOld::ModuleOculusOld()
{
	hmd = 0;
	fakehmd = ovrHmd_None;

	memset(&rtex, 0, sizeof rtex);
	rtex.fbscale = 1.0f;

	win_width = win_height = -1;
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

	if(!ovr_Initialize(0)) {
		print_error("failed to initialize libovr\n");
		return false;
	}

	fakehmd = parse_hmdtype(getenv("GOATVR_FAKEHMD"));
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
	return "oculus_old";
}

bool ModuleOculusOld::detect()
{
	print_info("running HMD detection\n");

	if(ovrHmd_Detect() <= 0 && !fakehmd) {
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

	if(fakehmd) {
		if(!(hmd = ovrHmd_CreateDebug(fakehmd))) {
			print_error("failed to create oculus debug HMD\n");
			return;
		}
	} else {
		if(!(hmd = ovrHmd_Create(0))) {
			print_error("failed to create oculus HMD\n");
			return;
		}
	}

	if(win_width == -1) {
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		win_width = vp[2];
		win_height = vp[3];
	}

	// get some useful values
	eye_height = ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, 1.65);


	// *attempt* to go fullscreen on the rift
	if(!fakehmd) {
		hmd_fullscreen(hmd->WindowsPos.x, hmd->WindowsPos.y);
	}

	// enable position and rotation tracking
	ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection
			| ovrTrackingCap_Position, 0);

	// fill the ovrGLConfig structure
	memset(&ovr_glcfg, 0, sizeof ovr_glcfg);
	ovr_glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	ovr_glcfg.OGL.Header.BackBufferSize.w = hmd->Resolution.h;
	ovr_glcfg.OGL.Header.BackBufferSize.h = hmd->Resolution.w;
	ovr_glcfg.OGL.Header.Multisample = 1;
#ifdef OVR_OS_WIN32
	ovr_glcfg.OGL.Window = GetActiveWindow();
	ovr_glcfg.OGL.DC = wglGetCurrentDC();
	if(!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop)) {
		ovrHmd_AttachToWindow(hmd, glcfg.OGL.Window, 0, 0);
	}
#endif
#ifdef OVR_OS_LINUX
	ovr_glcfg.OGL.Disp = glXGetCurrentDisplay();
#endif

	// enable low persistence and dynamic prediction
	unsigned int hmd_caps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	ovrHmd_SetEnabledCaps(hmd, hmd_caps);
	// configure SDK-rendering and enable display overdrive and timewarp
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
		ovrHmd_RecenterPose(hmd);
	}
}

void ModuleOculusOld::update()
{
	ovrVector3f eye_offs[2] = {
		ovr_rdesc[0].HmdToEyeViewOffset,
		ovr_rdesc[1].HmdToEyeViewOffset
	};

	ovrTrackingState tstate;
	ovrHmd_GetEyePoses(hmd, 0, eye_offs, ovr_poses, &tstate);

	for(int i=0; i<2; i++) {
		ovrVector3f pos = ovr_poses[i].Position;
		ovrQuatf rot = ovr_poses[i].Orientation;

		eye_pos[i] = Vec3(pos.x, pos.y, pos.z);
		eye_rot[i] = Quat(rot.x, rot.y, rot.z, rot.w);

		Mat4 rmat = eye_rot[i].calc_matrix();
		Mat4 tmat;
		tmat.translation(-eye_pos[i]);
		if(origin_mode == GOATVR_FLOOR) {
			tmat.translate(0, -eye_height, 0);
		}
		eye_inv_xform[i] = tmat * rmat;

		// TODO eye_xform

	}
	// TODO do we have to translate by eye_offs?
}

void ModuleOculusOld::set_fbsize(int width, int height, float fbscale)
{
	rtex.fbscale = fbscale;
	// this is only used for the mirror texture
	win_width = width;
	win_height = height;
}

RenderTexture *ModuleOculusOld::get_render_texture()
{
	ovrSizei texsz[2];
	for(int i=0; i<2; i++) {
		ovrEyeType eye = (ovrEyeType)i;
		texsz[i] = ovrHmd_GetFovTextureSize(hmd, eye, hmd->DefaultEyeFov[i], rtex.fbscale);

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

	if(!rtex.tex) {
		glGenTextures(1, &rtex.tex);
		glBindTexture(GL_TEXTURE_2D, rtex.tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		rtex.tex_width = -1; // invalidate width to force a tex rebuild
	}

	// recreate the texture if necessary
	if(rtex.tex_width != texwidth || rtex.tex_height != texheight) {
		print_info("creating %dx%d texture for %dx%d framebuffer\n", texwidth, texheight, fbwidth, fbheight);
		glBindTexture(GL_TEXTURE_2D, rtex.tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texwidth, texheight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		rtex.tex_width = texwidth;
		rtex.tex_height = texheight;
	}

	rtex.width = fbwidth;
	rtex.height = fbheight;

	// prepare the ovrGLTexture
	for(int i=0; i<2; i++) {
		ovr_gltex[i].OGL.Header.API = ovrRenderAPI_OpenGL;
		ovr_gltex[i].OGL.Header.TextureSize.w = texwidth;
		ovr_gltex[i].OGL.Header.TextureSize.h = texheight;
		ovr_gltex[i].OGL.Header.RenderViewport.Pos.x = i == 0 ? 0 : fbwidth / 2;
		ovr_gltex[i].OGL.Header.RenderViewport.Pos.y = 0;
		ovr_gltex[i].OGL.Header.RenderViewport.Size = {rtex.eye_width[i], rtex.eye_height[i]};
		ovr_gltex[i].OGL.TexId = rtex.tex;
	}
	return &rtex;
}

void ModuleOculusOld::draw_start()
{
	ovrHmd_BeginFrame(hmd, 0);
}

void ModuleOculusOld::draw_done()
{
	ovrHmd_EndFrame(hmd, ovr_poses, &ovr_gltex[0].Texture);
}

Mat4 ModuleOculusOld::get_view_matrix(int eye)
{
	return eye_inv_xform[eye];
}

Mat4 ModuleOculusOld::get_proj_matrix(int eye, float znear, float zfar)
{
	return ovr_matrix(ovrMatrix4f_Projection(ovr_rdesc[eye].Fov, znear, zfar,
			ovrProjection_RightHanded | ovrProjection_ClipRangeOpenGL));
}

static Mat4 ovr_matrix(const ovrMatrix4f &mat)
{
	return Mat4(mat.M[0][0], mat.M[1][0], mat.M[2][0], mat.M[3][0],
			mat.M[0][1], mat.M[1][1], mat.M[2][1], mat.M[3][1],
			mat.M[0][2], mat.M[1][2], mat.M[2][2], mat.M[3][2],
			mat.M[0][3], mat.M[1][3], mat.M[2][3], mat.M[3][3]);
}

static ovrHmdType parse_hmdtype(const char *s)
{
	static const struct { const char *name; ovrHmdType type; } hmds[] = {
		{ "dk1", ovrHmd_DK1 },
		{ "dk2", ovrHmd_DK2 },
		{ "dkhd", ovrHmd_DKHD },
		{ "blackstar", ovrHmd_BlackStar },
		{ "cb", ovrHmd_CB },
		{ 0, ovrHmd_None }
	};

	for(int i=0; hmds[i].name; i++) {
		if(strcasecmp(s, hmds[i].name) == 0) {
			return hmds[i].type;
		}
	}
	return s ? ovrHmd_DK2 : ovrHmd_None;
}

#else

#include "module.h"
// this expands to an empty register_mod_oculus() function
NOREG_MODULE(oculus_old)

#endif	// USE_MOD_OCULUS_OLD
