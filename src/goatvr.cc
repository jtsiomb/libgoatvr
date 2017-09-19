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
#include "inpman.h"
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

// action state for each hand
static bool action[GOATVR_NUM_ACTIONS][2];

// user information
static float user_eye_height = 1.65f;
static goatvr_user_gender user_gender = GOATVR_USER_UNKNOWN;

static float units_scale = 1.0f;

extern "C" {

int goatvr_init()
{
	if(!init_opengl()) {
		fprintf(stderr, "goatvr: opengl init failed\n");
		return -1;
	}
	register_modules();
	goatvr_detect();
	return display_module ? 0 : -1;
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
	if(!display_module) {
		fprintf(stderr, "goatvr: can't enter VR mode, no usable rendering module found\n");
		return;
	}

	if(!start()) return;
	in_vr = true;

	// make sure any changes done while not in VR make it through to the module
	display_module->set_origin_mode(origin_mode);

	user_swap = display_module->should_swap();
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
	if(display_module) {
		display_module->set_origin_mode(mode);
	}
	origin_mode = mode;
}

goatvr_origin_mode goatvr_get_origin_mode()
{
	return origin_mode;
}

void goatvr_recenter()
{
	if(display_module) {
		display_module->recenter();
	}
}

int goatvr_have_headtracking()
{
	if(display_module) {
		return display_module->have_headtracking();
	}
	return 0;
}

int goatvr_have_handtracking()
{
	if(display_module) {
		return display_module->have_handtracking();
	}
	return 0;
}

int goatvr_hand_active(int idx)
{
	if(display_module) {
		return display_module->hand_active(idx);
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
	if(display_module) {
		display_module->set_fbsize(width, height, scale);
	}
	cur_fbwidth = width;
	cur_fbheight = height;
	cur_fbscale = scale;
}

float goatvr_get_fb_scale()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->fbscale : 1.0f;
	}
	return cur_fbscale;
}

int goatvr_get_fb_width()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->width : 0;
	}
	return cur_fbwidth;
}

int goatvr_get_fb_height()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->height : 0;
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_width(int eye)
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->eye_width[eye] : 0;
	}
	return cur_fbwidth / 2;
}

int goatvr_get_fb_eye_height(int eye)
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->eye_height[eye] : 0;
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_xoffset(int eye)
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->eye_xoffs[eye] : 0;
	}
	return eye == GOATVR_LEFT ? 0 : cur_fbwidth / 2;
}

int goatvr_get_fb_eye_yoffset(int eye)
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->eye_yoffs[eye] : 0;
	}
	return 0;
}

unsigned int goatvr_get_fb_texture()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->tex : 0;
	}
	return 0;
}

int goatvr_get_fb_texture_width()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->tex_width : 0;
	}
	return next_pow2(cur_fbwidth);
}

int goatvr_get_fb_texture_height()
{
	if(display_module) {
		RenderTexture *rtex = display_module->get_render_texture();
		return rtex ? rtex->tex_height : 0;
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
	RenderTexture *rtex = display_module ? display_module->get_render_texture() : 0;
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
	if(display_module) {
		display_module->get_view_matrix(vmat[eye], eye);
		return vmat[eye][0];
	}
	return ident_mat;
}

float *goatvr_projection_matrix(int eye, float znear, float zfar)
{
	static Mat4 pmat[2];
	if(display_module) {
		display_module->get_proj_matrix(pmat[eye], eye, znear, zfar);
		return pmat[eye][0];
	}
	return ident_mat;
}

void goatvr_draw_start(void)
{
	display_module->draw_start(); // this needs to be called before update_fbo for oculus

	update_fbo();
	if(fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	update();	// this needs to be called *after* draw_start for oculus_old
}

void goatvr_draw_eye(int eye)
{
	if(!display_module) return;

	goatvr_viewport(eye);
	display_module->draw_eye(eye);
}

void goatvr_draw_done()
{
	if(!display_module) return;

	if(fbo) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	display_module->draw_mirror();

	display_module->draw_done();
}

int goatvr_should_swap()
{
	return user_swap ? 1 : 0;
}

// ---- input device handling ----

void goatvr_head_position(float *pos)
{
	Vec3 v;
	if(display_module) {
		v = display_module->get_head_position();
	}
	pos[0] = v.x;
	pos[1] = v.y;
	pos[2] = v.z;
}

void goatvr_head_orientation(float *quat)
{
	Quat q;
	if(display_module) {
		q = display_module->get_head_orientation();
	}
	quat[0] = q.x;
	quat[1] = q.y;
	quat[2] = q.z;
	quat[3] = q.w;
}

float *goatvr_head_matrix(void)
{
	static Mat4 mat;
	if(display_module) {
		display_module->get_head_matrix(mat);
		return mat[0];
	}
	return ident_mat;
}

void goatvr_hand_position(int hand, float *pos)
{
	Vec3 v;
	if(display_module) {
		v = display_module->get_hand_position(hand);
	}
	pos[0] = v.x;
	pos[1] = v.y;
	pos[2] = v.z;
}

void goatvr_hand_orientation(int hand, float *quat)
{
	Quat q;
	if(display_module) {
		q = display_module->get_hand_orientation(hand);
	}
	quat[0] = q.x;
	quat[1] = q.y;
	quat[2] = q.z;
	quat[3] = q.w;
}

float *goatvr_hand_matrix(int hand)
{
	static Mat4 mat;
	if(display_module) {
		display_module->get_hand_matrix(mat, hand);
		return mat[0];
	}
	return ident_mat;
}

int goatvr_action(int hand, int act)
{
	if(act < 0 || act >= GOATVR_NUM_ACTIONS) {
		return 0;
	}
	return action[act][hand];
}

int goatvr_num_buttons(void)
{
	return inp_num_buttons();
}

const char *goatvr_button_name(int bnidx)
{
	InputButton *bn = inp_get_button(bnidx);
	return bn ? bn->name.c_str() : 0;
}

int goatvr_button_state(int bnidx)
{
	InputButton *bn = inp_get_button(bnidx);
	if(!bn) return 0;

	return (bn->module->get_button_state(1 << bn->mod_idx) >> bn->mod_idx) & 1;
}

int goatvr_lookup_button(const char *name)
{
	InputButton *bn = inp_find_button(name);
	return bn ? bn->idx : -1;
}

int goatvr_num_axes(void)
{
	return inp_num_axes();
}

const char *goatvr_axis_name(int axis_idx)
{
	InputAxis *axis = inp_get_axis(axis_idx);
	return axis ? axis->name.c_str() : 0;
}

float goatvr_axis_value(int axis_idx)
{
	InputAxis *axis = inp_get_axis(axis_idx);
	if(!axis) return 0.0f;

	return axis->module->get_axis_value(axis->mod_idx);
}

int goatvr_lookup_axis(const char *name)
{
	InputAxis *axis = inp_find_axis(name);
	return axis ? axis->idx : -1;
}

int goatvr_num_sticks(void)
{
	return inp_num_sticks();
}

const char *goatvr_stick_name(int stick_idx)
{
	InputStick *stick = inp_get_stick(stick_idx);
	return stick ? stick->name.c_str() : 0;
}

void goatvr_stick_pos(int stick_idx, float *pos)
{
	InputStick *stick = inp_get_stick(stick_idx);
	if(stick) {
		Vec2 p = stick->module->get_stick_pos(stick->mod_idx);
		pos[0] = p.x;
		pos[1] = p.y;
	}
}

int goatvr_lookup_stick(const char *name)
{
	InputStick *stick = inp_find_stick(name);
	return stick ? stick->idx : -1;
}

// ---- module management ----

int goatvr_activate_module(goatvr_module *mod)
{
	activate(mod);
	return 0;
}

int goatvr_deactivate_module(goatvr_module *mod)
{
	deactivate(mod);
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

const char *goatvr_module_name(goatvr_module *mod)
{
	return mod->get_name();
}

enum goatvr_module_type goatvr_module_type(goatvr_module *mod)
{
	return mod->get_type();
}

int goatvr_module_active(goatvr_module *mod)
{
	return mod->active();
}

int goatvr_module_usable(goatvr_module *mod)
{
	return mod->usable();
}

int goatvr_module_headtracking(goatvr_module *mod)
{
	return mod->have_headtracking();
}

int goatvr_module_handtracking(goatvr_module *mod)
{
	return mod->have_handtracking();
}

int goatvr_module_num_buttons(goatvr_module *mod)
{
	return mod->num_buttons();
}

int goatvr_module_num_axes(goatvr_module *mod)
{
	return mod->num_axes();
}

int goatvr_module_num_sticks(goatvr_module *mod)
{
	return mod->num_sticks();
}

const char *goatvr_module_button_name(goatvr_module *mod, int bn)
{
	if(bn < 0 || bn >= mod->num_buttons()) {
		return 0;
	}
	return mod->get_button_name(bn);
}

const char *goatvr_module_axis_name(goatvr_module *mod, int axis)
{
	if(axis < 0 || axis >= mod->num_axes()) {
		return 0;
	}
	return mod->get_axis_name(axis);
}

const char *goatvr_module_stick_name(goatvr_module *mod, int stick)
{
	if(stick < 0 || stick >= mod->num_sticks()) {
		return 0;
	}
	return mod->get_stick_name(stick);
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

void goatvr::set_action(int which, int hand, bool value)
{
	action[which][hand] = value;
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

	if(!display_module) {
		return false;
	}

	RenderTexture *rtex = display_module->get_render_texture();
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
