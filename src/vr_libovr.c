#ifdef WIN32
#define OVR_OS_WIN32
#endif
#ifdef __APPLE__
#define OVR_OS_MAC
#endif

#include "vr_impl.h"

#ifdef USE_LIBOVR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "opt.h"

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

#define DISABLE_RETARDED_HEALTH_WARNING

/* just dropping the prototype here to avoid including CAPI_HSWDisplay.h */
OVR_EXPORT void ovrhmd_EnableHSWDisplaySDKRender(ovrHmd hmd, ovrBool enabled);

static ovrHmd hmd;
static void *optdb;
static ovrEyeRenderDesc eye_render_desc[2];
static ovrSizei eye_res[2];
static ovrGLTexture eye_tex[2];
static ovrFovPort eye_fov[2];
static ovrPosef pose[2];
static int deferred_init_done;

static int init(void)
{
	int i, num_hmds;
	int use_fake = 0;

	if(!ovr_Initialize()) {
		return -1;
	}
	printf("initialized LibOVR %s\n", ovr_GetVersionString());

	if(!(num_hmds = ovrHmd_Detect())) {
		if(getenv("VR_LIBOVR_FAKE")) {
			use_fake = 1;
			num_hmds = 1;
		} else {
			ovr_Shutdown();
			return -1;
		}
	}
	printf("%d Oculus HMD(s) found\n", num_hmds);

	hmd = 0;
	for(i=0; i<num_hmds; i++) {
		ovrHmd h = use_fake ? ovrHmd_CreateDebug(ovrHmd_DK2) : ovrHmd_Create(i);
		if(!h) {
			break;
		}
		printf(" [%d]: %s - %s\n", i, h->Manufacturer, h->ProductName);

		if(!hmd) {
			hmd = h;
		} else {
			ovrHmd_Destroy(h);
		}
	}

	if(!hmd) {
		fprintf(stderr, "failed to initialize any Oculus HMDs\n");
		return -1;
	}

	ovrHmd_ConfigureTracking(hmd, 0xffffffff, 0);

	eye_fov[0] = hmd->DefaultEyeFov[0];
	eye_fov[1] = hmd->DefaultEyeFov[1];

	eye_res[0] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, eye_fov[0], 1.0);
	eye_res[1] = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, eye_fov[1], 1.0);

	/* create the options database */
	if((optdb = create_options())) {
		set_option_int(optdb, VR_OPT_DISPLAY_WIDTH, hmd->Resolution.w);
		set_option_int(optdb, VR_OPT_DISPLAY_HEIGHT, hmd->Resolution.h);
		set_option_int(optdb, VR_OPT_LEYE_XRES, eye_res[0].w);
		set_option_int(optdb, VR_OPT_LEYE_YRES, eye_res[0].h);
		set_option_int(optdb, VR_OPT_REYE_XRES, eye_res[1].w);
		set_option_int(optdb, VR_OPT_REYE_YRES, eye_res[1].h);
		set_option_float(optdb, VR_OPT_EYE_HEIGHT, ovrHmd_GetFloat(hmd, OVR_KEY_EYE_HEIGHT, OVR_DEFAULT_EYE_HEIGHT));
		set_option_float(optdb, VR_OPT_IPD, ovrHmd_GetFloat(hmd, OVR_KEY_IPD, OVR_DEFAULT_IPD));
		set_option_int(optdb, VR_OPT_WIN_XOFFS, hmd->WindowsPos.x);
		set_option_int(optdb, VR_OPT_WIN_YOFFS, hmd->WindowsPos.y);
	}

	deferred_init_done = 0;
	return 0;
}

static void deferred_init(void)
{
	union ovrGLConfig glcfg;
	unsigned int dcaps;
	void *win = 0;

	deferred_init_done = 1;

	memset(&glcfg, 0, sizeof glcfg);
	glcfg.OGL.Header.API = ovrRenderAPI_OpenGL;
	glcfg.OGL.Header.RTSize = hmd->Resolution;
	glcfg.OGL.Header.Multisample = 1;
#ifdef WIN32
	win = GetActiveWindow();
	/*glcfg.OGL.Window = win;
	glcfg.OGL.DC = wglGetCurrentDC();
	assert(glcfg.OGL.Window);
	assert(glcfg.OGL.DC);*/
#endif

	if(!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop)) {
		ovrHmd_AttachToWindow(hmd, win, 0, 0);
		printf("running in \"direct-to-rift\" mode\n");
	} else {
		printf("running in \"extended desktop\" mode\n");
	}
	ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);

	dcaps = ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp |
		ovrDistortionCap_Overdrive | ovrDistortionCap_NoRestore;

	if(!ovrHmd_ConfigureRendering(hmd, &glcfg.Config, dcaps, eye_fov, eye_render_desc)) {
		fprintf(stderr, "failed to configure LibOVR distortion renderer\n");
	}

#ifdef DISABLE_RETARDED_HEALTH_WARNING
	ovrhmd_EnableHSWDisplaySDKRender(hmd, 0);
#endif
}

static void cleanup(void)
{
	if(hmd) {
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}
	destroy_options(optdb);
}

static int set_option(const char *opt, enum opt_type type, void *valp)
{
	switch(type) {
	case OTYPE_INT:
		set_option_int(optdb, opt, *(int*)valp);
		break;

	case OTYPE_FLOAT:
		set_option_float(optdb, opt, *(float*)valp);
		break;
	}
	return 0;
}

static int get_option(const char *opt, enum opt_type type, void *valp)
{
	switch(type) {
	case OTYPE_INT:
		return get_option_int(optdb, opt, valp);
	case OTYPE_FLOAT:
		return get_option_float(optdb, opt, valp);
	}
	return -1;
}

static void translation(int eye, float *vec)
{
	if(!hmd) {
		vec[0] = vec[1] = vec[2] = 0;
		return;
	}

	pose[eye] = ovrHmd_GetEyePose(hmd, eye == VR_EYE_LEFT ? ovrEye_Left : ovrEye_Right);
	vec[0] = pose[eye].Position.x + eye_render_desc[eye].ViewAdjust.x;
	vec[1] = pose[eye].Position.y + eye_render_desc[eye].ViewAdjust.y;
	vec[2] = pose[eye].Position.z + eye_render_desc[eye].ViewAdjust.z;
}

static void rotation(int eye, float *quat)
{
	if(!hmd) {
		quat[0] = quat[1] = quat[2] = 0.0f;
		quat[3] = 1.0f;
		return;
	}

	pose[eye] = ovrHmd_GetEyePose(hmd, eye == VR_EYE_LEFT ? ovrEye_Left : ovrEye_Right);
	quat[0] = pose[eye].Orientation.x;
	quat[1] = pose[eye].Orientation.y;
	quat[2] = pose[eye].Orientation.z;
	quat[3] = pose[eye].Orientation.w;
}

static const float idmat[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

static void proj_matrix(int eye, float znear, float zfar, float *mat)
{
	int i, j;
	ovrMatrix4f vmat;

	if(!hmd) {
		memcpy(mat, idmat, sizeof idmat);
		return;
	}

	vmat = ovrMatrix4f_Projection(eye_render_desc[eye].Fov, znear, zfar, 1);

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			*mat++ = vmat.M[j][i];
		}
	}
}

static int new_frame = 1;

static void begin(int eye)
{
	if(!hmd) return;

	if(!deferred_init_done) {
		deferred_init();
	}

	if(new_frame) {
		ovrHmd_BeginFrame(hmd, 0);
		new_frame = 0;
	}
}

static int present(void)
{
	if(!hmd) return 0;

	ovrHmd_EndFrame(hmd, pose, &eye_tex[0].Texture);
	new_frame = 1;

	return 1;
}

static void set_eye_texture(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax)
{
	ovrSizei texsz;
	ovrRecti rect;

	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texsz.w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texsz.h);

	rect.Pos.x = (int)(umin * texsz.w);
	rect.Pos.y = (int)((vmin + 1.0 - vmax) * texsz.h);
	rect.Size.w = (int)((umax - umin) * texsz.w);
	rect.Size.h = (int)((vmax - vmin) * texsz.h);

	eye_tex[eye].OGL.Header.API = ovrRenderAPI_OpenGL;
	eye_tex[eye].OGL.Header.TextureSize = texsz;
	eye_tex[eye].OGL.Header.RenderViewport = rect;
	eye_tex[eye].OGL.TexId = tex;
}

static void recenter(void)
{
	if(hmd) {
		ovrHmd_RecenterPose(hmd);
	}
}

struct vr_module *vr_module_libovr(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "libovr";
		m.init = init;
		m.cleanup = cleanup;
		m.set_option = set_option;
		m.get_option = get_option;
		m.translation = translation;
		m.rotation = rotation;
		m.proj_matrix = proj_matrix;
		m.begin = begin;
		m.present = present;
		m.set_eye_texture = set_eye_texture;
		m.recenter = recenter;
	}
	return &m;
}

#else	/* no libovr */

static int init(void)
{
	return -1;
}

struct vr_module *vr_module_libovr(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "libovr";
		m.init = init;
	}
	return &m;
}

#endif	/* USE_LIBOVR */
