#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opengl.h"
#include "vr.h"
#include "vr_impl.h"
#include "mathutil.h"


static void fallback_present(void);


static struct vr_module *vrm;
static float idmat[] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
static float fbtex_rect[] = {
	0, 0, 1, 1
};

static void *defopt;	/* default options db */

static struct {
	float umin, umax, vmin, vmax;
	int tex;
} rtarg[2];


int vr_init(void)
{
	int i, nmodules;
	char *vrmod_env;

	/* create the default options database */
	if(!defopt && (defopt = create_options())) {
		set_option_float(defopt, VR_EYE_HEIGHT, 1.675);
		set_option_float(defopt, VR_IPD, 0.064);
	}

	if(vrm) {
		vr_shutdown();
	}

	vr_init_modules();

	nmodules = vr_get_num_modules();
	for(i=0; i<nmodules; i++) {
		struct vr_module *m = vr_get_module(i);
		if(m->init() != -1) {
			/* add to the active modules array */
			vr_activate_module(i);
		}
	}

	if(!vr_get_num_active_modules()) {
		return -1;
	}

	if((vrmod_env = getenv("VR_MODULE"))) {
		vr_use_module_named(vrmod_env);
	} else {
		vr_use_module(0);
	}
	return 0;
}

void vr_shutdown(void)
{
	vr_clear_modules();
	vrm = 0;
	fbtex_rect[0] = fbtex_rect[1] = 0;
	fbtex_rect[2] = fbtex_rect[3] = 1;
}

int vr_module_count(void)
{
	return vr_get_num_active_modules();
}

const char *vr_module_name(int idx)
{
	struct vr_module *m = vr_get_active_module(idx);
	if(!m) {
		return 0;
	}
	return m->name;
}

int vr_use_module(int idx)
{
	if(idx >= 0 && idx < vr_get_num_active_modules()) {
		struct vr_module *m = vr_get_active_module(idx);
		if(m != vrm) {
			vrm = m;
			printf("using vr module: %s\n", vrm->name);
		}
		return 0;
	}
	return -1;
}

int vr_use_module_named(const char *name)
{
	int i, count = vr_get_num_active_modules();

	for(i=0; i<count; i++) {
		struct vr_module *m = vr_get_active_module(i);
		if(strcmp(m->name, name) == 0) {
			return vr_use_module(i);
		}
	}
	return -1;
}

void vr_seti(const char *optname, int val)
{
	if(vrm && vrm->set_option) {
		vrm->set_option(optname, OTYPE_INT, &val);
	} else {
		set_option_int(defopt, optname, val);
	}
}

void vr_setf(const char *optname, float val)
{
	if(vrm && vrm->set_option) {
		vrm->set_option(optname, OTYPE_FLOAT, &val);
	} else {
		set_option_float(defopt, optname, val);
	}
}

int vr_geti(const char *optname)
{
	int res = 0;

	if(!vrm || !vrm->get_option || vrm->get_option(optname, OTYPE_INT, &res) == -1) {
		get_option_int(defopt, optname, &res);	/* fallback */
	}
	return res;
}

float vr_getf(const char *optname)
{
	float res = 0.0f;

	if(!vrm || !vrm->get_option || vrm->get_option(optname, OTYPE_FLOAT, &res) == -1) {
		get_option_float(defopt, optname, &res);	/* fallback */
	}
	return res;
}


int vr_view_translation(int eye, float *vec)
{
	if(vrm && vrm->translation) {
		vrm->translation(eye, vec);
		return 1;
	}
	vec[0] = vec[1] = vec[2] = 0.0f;
	return 0;
}

int vr_view_rotation(int eye, float *quat)
{
	if(vrm && vrm->rotation) {
		vrm->rotation(eye, quat);
		return 1;
	}
	quat[0] = quat[1] = quat[2] = 0.0f;
	quat[3] = 1.0f;
	return 0;
}

int vr_view_matrix(int eye, float *mat)
{
	int have_trans, have_rot;
	float offs[3], quat[4];
	float rmat[16], tmat[16];

	if(vrm && vrm->view_matrix) {
		vrm->view_matrix(eye, mat);
		return 1;
	}

	have_trans = vr_view_translation(eye, offs);
	have_rot = vr_view_rotation(eye, quat);

	if(!have_trans && !have_rot) {
		memcpy(mat, idmat, sizeof idmat);
		return 0;
	}

	offs[0] = -offs[0];
	offs[1] = -offs[1];
	offs[2] = -offs[2];

	vrimp_translation_matrix(offs, tmat);
	vrimp_rotation_matrix(quat, rmat);
	vrimp_mult_matrix(mat, tmat, rmat);
	return 1;
}

int vr_proj_matrix(int eye, float znear, float zfar, float *mat)
{
	if(vrm && vrm->proj_matrix) {
		vrm->proj_matrix(eye, znear, zfar, mat);
		return 1;
	}
	memcpy(mat, idmat, sizeof idmat);
	return 0;
}

void vr_begin(int eye)
{
	if(vrm && vrm->begin) {
		vrm->begin(eye);
	}
}

void vr_end(void)
{
	if(vrm && vrm->end) {
		vrm->end();
	}
}

int vr_swap_buffers(void)
{
	int res = 0;

	if(vrm && vrm->present) {
		res = vrm->present();
	}

	if(!res) {
		fallback_present();
		vrimp_swap_buffers();
	}
	return 0;
}

void vr_output_texture(unsigned int tex, float umin, float vmin, float umax, float vmax)
{
	float halfu = (umax + umin) * 0.5f;

	vr_output_texture_eye(VR_EYE_LEFT, tex, umin, vmin, halfu, vmax);
	vr_output_texture_eye(VR_EYE_RIGHT, tex, halfu, vmin, umax, vmax);
}

void vr_output_texture_eye(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax)
{
	if(vrm && vrm->set_eye_texture) {
		vrm->set_eye_texture(eye, tex, umin, vmin, umax, vmax);
	} else {
		rtarg[eye].tex = tex;
		rtarg[eye].umin = umin;
		rtarg[eye].umax = umax;
		rtarg[eye].vmin = vmin;
		rtarg[eye].vmax = vmax;
	}
}

void vr_recenter(void)
{
	if(vrm && vrm->recenter) {
		vrm->recenter();
	}
}

static void fallback_present(void)
{
	int i;

	glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_FOG);

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	for(i=0; i<2; i++) {
		float x0 = i == 0 ? -1 : 0;
		float x1 = i == 0 ? 0 : 1;

		glBindTexture(GL_TEXTURE_2D, rtarg[i].tex);

		glBegin(GL_QUADS);
		glTexCoord2f(rtarg[i].umin, rtarg[i].vmin);
		glVertex2f(x0, -1);
		glTexCoord2f(rtarg[i].umax, rtarg[i].vmin);
		glVertex2f(x1, -1);
		glTexCoord2f(rtarg[i].umax, rtarg[i].vmax);
		glVertex2f(x1, 1);
		glTexCoord2f(rtarg[i].umin, rtarg[i].vmax);
		glVertex2f(x0, 1);
		glEnd();
	}

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}
