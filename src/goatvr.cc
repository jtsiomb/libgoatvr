/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2017  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "opengl.h"
#include "goatvr_impl.h"
#include "modman.h"
#include "autocfg.h"

using namespace goatvr;

namespace goatvr {
void register_modules();	// auto-generated function during build
}

static bool update_fbo();

static goatvr_origin_mode origin_mode = GOATVR_FLOOR;

static bool in_vr;
// user-supplied framebuffer properties
static int cur_fbwidth, cur_fbheight;
static float cur_fbscale = 1.0f;

static unsigned int fbo;
static unsigned int zbuf;
static int fbo_width, fbo_height;

static bool user_swap = true;

// user information
static float user_eye_height = 1.65f;
static goatvr_user_gender user_gender = GOATVR_USER_UNKNOWN;

static float units_scale = 1.0f;

static std::vector<Source*> sources;

extern "C" {

int goatvr_init()
{
	if(!init_opengl()) {
		fprintf(stderr, "goatvr: opengl init failed\n");
		return -1;
	}
	register_modules();
	goatvr_detect();
	return render_module ? 0 : -1;
}

void goatvr_shutdown()
{
	goatvr_stopvr();
	destroy_modules();

	if(fbo) {
		glDeleteFramebuffers(1, &fbo);
		glDeleteRenderbuffers(1, &zbuf);
	}
}

void goatvr_detect()
{
	printf("goatvr: module detection running for %d total modules ...\n", get_num_modules());
	detect();

	// automatic module selection and activation (in autocfg.cc)
	activate_module();
}

void goatvr_startvr()
{
	if(in_vr) return;

	// make sure we have at least one rendering module
	if(!render_module) {
		fprintf(stderr, "goatvr: can't enter VR mode, no usable rendering module found\n");
		return;
	}

	start();
	in_vr = true;

	// make sure any changes done while not in VR make it through to the module
	render_module->set_origin_mode(origin_mode);

	user_swap = render_module->should_swap();

	/* configure initial tracking sources (if the render module doesn't provide tracking
	 * or if the user requests it through an environment variable) (in autocfg.cc).
	 */
	configure_tracking();
}

void goatvr_stopvr()
{
	if(in_vr) stop();
}

int goatvr_invr()
{
	return in_vr ? 1 : 0;
}

void goatvr_set_origin_mode(goatvr_origin_mode mode)
{
	if(render_module) {
		render_module->set_origin_mode(mode);
	}
	origin_mode = mode;
}

goatvr_origin_mode goatvr_get_origin_mode()
{
	return origin_mode;
}

void goatvr_recenter()
{
	if(render_module) {
		render_module->recenter();
	}
}

int goatvr_have_headtracking()
{
	if(render_module) {
		return render_module->have_head_tracking();
	}
	return 0;
}

int goatvr_have_handtracking()
{
	if(render_module) {
		return render_module->have_hand_tracking();
	}
	return 0;
}

void goatvr_set_default_tracker(void)
{
	if(render_module) {
		render_module->set_default_sources();
	}
}

void goatvr_set_head_tracker(goatvr_source *s)
{
	if(render_module) {
		render_module->set_head_source(s);
	}
}

void goatvr_set_hand_tracker(int idx, goatvr_source *s)
{
	if(render_module) {
		render_module->set_hand_source(idx, s);
	}
}

goatvr_source *goatvr_get_head_tracker(void)
{
	if(render_module) {
		return render_module->get_head_source();
	}
	return 0;
}

goatvr_source *goatvr_get_hand_tracker(int idx)
{
	if(render_module) {
		return render_module->get_hand_source(idx);
	}
	return 0;
}

void goatvr_set_units_scale(float us)
{
	units_scale = us;
}

float goatvr_get_units_scale(void)
{
	return units_scale;
}

// ---- rendering ----

void goatvr_set_fb_size(int width, int height, float scale)
{
	if(render_module) {
		render_module->set_fbsize(width, height, scale);
	}
	cur_fbwidth = width;
	cur_fbheight = height;
	cur_fbscale = scale;
}

float goatvr_get_fb_scale()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->fbscale;
	}
	return cur_fbscale;
}

int goatvr_get_fb_width()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->width;
	}
	return cur_fbwidth;
}

int goatvr_get_fb_height()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->height;
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_width(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_width[eye];
	}
	return cur_fbwidth / 2;
}

int goatvr_get_fb_eye_height(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_height[eye];
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_xoffset(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_xoffs[eye];
	}
	return eye == GOATVR_LEFT ? 0 : cur_fbwidth / 2;
}

int goatvr_get_fb_eye_yoffset(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_yoffs[eye];
	}
	return 0;
}

unsigned int goatvr_get_fb_texture()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->tex;
	}
	return 0;
}

int goatvr_get_fb_texture_width()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->tex_width;
	}
	return next_pow2(cur_fbwidth);
}

int goatvr_get_fb_texture_height()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->tex_height;
	}
	return next_pow2(cur_fbheight);
}

unsigned int goatvr_get_fbo(void)
{
	update_fbo();
	return fbo;
}


void goatvr_viewport(int eye)
{
	RenderTexture *rtex = render_module ? render_module->get_render_texture() : 0;
	if(rtex) {
		glViewport(rtex->eye_xoffs[eye], rtex->eye_yoffs[eye], rtex->eye_width[eye], rtex->eye_height[eye]);
	} else {
		glViewport(eye == 0 ? 0 : cur_fbwidth / 2, 0, cur_fbwidth / 2, cur_fbheight);
	}
}

static float ident_mat[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

float *goatvr_view_matrix(int eye)
{
	static Mat4 vmat[2];
	if(render_module) {
		vmat[eye] = render_module->get_view_matrix(eye);
		return vmat[eye][0];
	}
	return ident_mat;
}

float *goatvr_projection_matrix(int eye, float znear, float zfar)
{
	static Mat4 pmat[2];
	if(render_module) {
		pmat[eye] = render_module->get_proj_matrix(eye, znear, zfar);
		return pmat[eye][0];
	}
	return ident_mat;
}

void goatvr_draw_start(void)
{
	render_module->draw_start(); // this needs to be called before update_fbo for oculus

	update_fbo();
	if(fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	update();	// this needs to be called *after* draw_start for oculus_old
}

void goatvr_draw_eye(int eye)
{
	if(!render_module) return;

	goatvr_viewport(eye);
	render_module->draw_eye(eye);
}

void goatvr_draw_done()
{
	if(!render_module) return;

	if(fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	render_module->draw_mirror();

	render_module->draw_done();
}

int goatvr_should_swap()
{
	return user_swap ? 1 : 0;
}

// ---- input device handling ----
int goatvr_num_sources(void)
{
	return (int)sources.size();
}

goatvr_source *goatvr_get_source(int idx)
{
	return sources[idx];
}

goatvr_source *goatvr_find_source(const char *name)
{
	int num = goatvr_num_sources();
	for(int i=0; i<num; i++) {
		if(strcasecmp(goatvr_source_name(sources[i]), name) == 0) {
			return sources[i];
		}
	}
	return 0;
}

const char *goatvr_source_name(goatvr_source *dev)
{
	return dev->mod->get_source_name(dev->mod_data);
}

int goatvr_source_spatial(goatvr_source *dev)
{
	return dev->mod->is_source_spatial(dev->mod_data);
}

int goatvr_source_num_axes(goatvr_source *dev)
{
	return dev->mod->get_source_num_axes(dev->mod_data);
}

int goatvr_source_num_buttons(goatvr_source *dev)
{
	return dev->mod->get_source_num_buttons(dev->mod_data);
}

void goatvr_source_position(goatvr_source *dev, float *pos)
{
	Vec3 p = dev->mod->get_source_pos(dev->mod_data);
	pos[0] = p.x;
	pos[1] = p.y;
	pos[2] = p.z;
}

void goatvr_source_orientation(goatvr_source *dev, float *quat)
{
	Quat q = dev->mod->get_source_rot(dev->mod_data);
	quat[0] = q.x;
	quat[1] = q.y;
	quat[2] = q.z;
	quat[3] = q.w;
}

float *goatvr_source_matrix(goatvr_source *dev)
{
	return dev->xform[0];
}

// ---- module management ----

int goatvr_activate_module(goatvr_module *vrmod)
{
	activate(vrmod);
	return 0;
}

int goatvr_deactivate_module(goatvr_module *vrmod)
{
	deactivate(vrmod);
	return 0;
}

int goatvr_num_modules()
{
	return get_num_modules();
}

goatvr_module *goatvr_get_module(int idx)
{
	return get_module(idx);
}

goatvr_module *goatvr_find_module(const char *name)
{
	return find_module(name);
}

const char *goatvr_module_name(goatvr_module *vrmod)
{
	return vrmod->get_name();
}

int goatvr_module_active(goatvr_module *vrmod)
{
	return vrmod->active();
}

int goatvr_module_usable(goatvr_module *vrmod)
{
	return vrmod->usable();
}

// ---- user information ----

float goatvr_get_eye_height()
{
	return user_eye_height * units_scale;
}

goatvr_user_gender goatvr_get_user_gender()
{
	return user_gender;
}

// ----- utility functions ----

void goatvr_util_quat_to_matrix(float *mat, const float *quat)
{
	Quat q = Quat(quat[0], quat[1], quat[2], quat[3]);
	Mat4 qmat = q.calc_matrix();
	memcpy(mat, qmat[0], 16 * sizeof(float));
}

int goatvr_util_invert_matrix(float *inv, const float *mat)
{
	Mat4 m = Mat4(mat);
	bool res = m.inverse();
	memcpy(inv, m[0], 16 * sizeof(float));
	return res;
}

}	// extern "C"

void goatvr::add_source(Source *s)
{
	std::vector<Source*>::iterator it = std::find(sources.begin(), sources.end(), s);
	if(it == sources.end()) {
		sources.push_back(s);
	}
}

void goatvr::remove_source(Source *s)
{
	std::vector<Source*>::iterator it = std::find(sources.begin(), sources.end(), s);
	if(it != sources.end()) {
		sources.erase(it);
	}
}

void goatvr::set_user_eye_height(float height)
{
	user_eye_height = height / units_scale;
}

void goatvr::set_user_gender(goatvr_user_gender gender)
{
	user_gender = gender;
}

unsigned int goatvr::next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

void goatvr::calc_matrix(Mat4 &mat, const Vec3 &pos, const Quat &rot)
{
	Mat4 rmat = rot.calc_matrix();
	Mat4 tmat;
	tmat.translation(pos);
	mat = rmat * tmat;
}

void goatvr::calc_inv_matrix(Mat4 &mat, const Vec3 &pos, const Quat &rot)
{
	Mat4 rmat = rot.calc_matrix();
	rmat.transpose();
	Mat4 tmat;
	tmat.translation(-pos);
	mat = tmat * rmat;
}

static bool update_fbo()
{
	static unsigned int last_tex;	// last texture we got from Module::get_render_texture()

	if(!render_module) {
		return false;
	}

	RenderTexture *rtex = render_module->get_render_texture();
	if(!rtex) {
		return false;
	}

	if(!fbo) {
		glGenFramebuffers(1, &fbo);
		glGenRenderbuffers(1, &zbuf);
	}

	/* every time we call Module::get_render_texture() we might get a different texture
	 * make sure to re-bind it as the fbo color attachment if it changed
	 */
	if(last_tex != rtex->tex) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtex->tex, 0);
		last_tex = rtex->tex;
	}

	// resize fbo if necessary (or if it's the first time)
	if(fbo_width != rtex->tex_width || fbo_height != rtex->tex_height) {
		fbo_width = rtex->tex_width;
		fbo_height = rtex->tex_height;

		glBindRenderbuffer(GL_RENDERBUFFER, zbuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_width, fbo_height);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuf);

		GLenum fbst = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(fbst != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "goatvr: incomplete framebuffer! (status: %x)\n", (unsigned int)fbst);
			return false;
		}
	}
	return true;
}
