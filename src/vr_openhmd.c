#include "vr_impl.h"

#ifdef USE_OPENHMD
#include <stdio.h>
#include <stdlib.h>
#include <openhmd/openhmd.h>
#include "opt.h"

/* a noble spirit embiggens the framebuffer to avoid aliasing in the middle */
#define EMBIGGEN	1.5

static ohmd_context *ctx;
static ohmd_device *dev;
static void *optdb;
static int new_frame = 1;

static int disp_width, disp_height;
static float ipd;

static struct {
	unsigned int id;
	float umin, umax;
	float vmin, vmax;
} eye_tex[2];


static int init(void)
{
	int i, num_hmds;

	if(!(ctx = ohmd_ctx_create())) {
		fprintf(stderr, "failed to create OpenHMD context\n");
		ohmd_ctx_destroy(ctx);
		return -1;
	}
	if(!(num_hmds = ohmd_ctx_probe(ctx))) {
		fprintf(stderr, "no HMDs detected\n");
		return -1;
	}

	for(i=0; i<num_hmds; i++) {
		const char *vendor = ohmd_list_gets(ctx, i, OHMD_VENDOR);
		const char *product = ohmd_list_gets(ctx, i, OHMD_PRODUCT);
		const char *devpath = ohmd_list_gets(ctx, i, OHMD_PATH);

		printf("[%d] %s - %s (path: %s)\n", i, vendor, product, devpath);
	}

	printf("opening device 0\n");

	if(!(dev = ohmd_list_open_device(ctx, 0))) {
		fprintf(stderr, "failed to open device 0: %s\n", ohmd_ctx_get_error(ctx));
		return -1;
	}

	ohmd_device_geti(dev, OHMD_SCREEN_HORIZONTAL_SIZE, &disp_width);
	ohmd_device_geti(dev, OHMD_SCREEN_VERTICAL_SIZE, &disp_height);
	ohmd_device_getf(dev, OHMD_EYE_IPD, &ipd);

	if((optdb = create_options())) {
		set_option_int(optdb, VR_OPT_DISPLAY_WIDTH, disp_width);
		set_option_int(optdb, VR_OPT_DISPLAY_HEIGHT, disp_height);
		set_option_float(optdb, VR_OPT_IPD, ipd);

		set_option_int(optdb, VR_OPT_LEYE_XRES, (int)(disp_width / 2.0 * FB_EMBIGGEN));
		set_option_int(optdb, VR_OPT_LEYE_YRES, (int)(disp_height * FB_EMBIGGEN));
		set_option_int(optdb, VR_OPT_REYE_XRES, (int)(disp_width / 2.0 * FB_EMBIGGEN));
		set_option_int(optdb, VR_OPT_REYE_YRES, (int)(disp_height * FB_EMBIGGEN));
	}

	return 0;
}

static void cleanup(void)
{
	if(ctx) {
		ohmd_ctx_destroy(ctx);
	}
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

static int translation(int eye, float *vec)
{
	float xform[16];

	view_matrix(eye, xform);

	vec[0] = xform[3];
	vec[1] = xform[7];
	vec[2] = xform[11];
	return 0;
}

static int rotation(int eye, float *quat)
{
	if(!dev) {
		quat[0] = quat[1] = quat[2] = 0.0f;
		quat[3] = 1.0f;
		return -1;
	}

	ohmd_device_getf(dev, OHMD_ROTATION_QUAT, quat);
	return 0;
}


static void view_matrix(int eye, float *mat)
{
	ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX + eye, mat);
}

static void proj_matrix(int eye, float znear, float zfar, float *mat)
{
	ohmd_device_setf(dev, OHMD_PROJECTION_ZNEAR, znear);
	ohmd_device_setf(dev, OHMD_PROJECTION_ZFAR, zfar);
	ohmd_device_getf(dev, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX + eye, mat);
}

static void begin(int eye)
{
	if(new_frame) {
		ohmd_ctx_update(ctx);
		new_frame = 0;
	}
}

static int present(void)
{
	new_frame = 1;
	return 0;
}

static void set_eye_texture(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax)
{
	eye_tex[eye].id = tex;
	eye_tex[eye].umin = umin;
	eye_tex[eye].umax = umax;
	eye_tex[eye].vmin = vmin;
	eye_tex[eye].vmax = vmax;
}

static void recenter(void)
{
	/* TODO does OHMD support the magnetometer? */
}


struct vr_module *vr_module_openhmd(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "openhmd";
		m.init = init;
		m.cleanup = cleanup;
		m.set_option = set_option;
		m.get_option = get_option;
		m.view_matrix = view_matrix;
		m.proj_matrix = proj_matrix;
		m.begin = begin;
		m.present = present;
		m.set_eye_texture = set_eye_texture;
		m.recenter = recenter;
	}
}

#else	/* don't use OpenHMD */

static int init(void)
{
	return -1;
}

struct vr_module *vr_module_openhmd(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "openhmd";
		m.init = init;
	}
	return &m;
}

#endif	/* USE_OPENHMD */
