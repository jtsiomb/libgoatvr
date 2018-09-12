/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2018  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef LIBGOATVR_H_
#define LIBGOATVR_H_

#ifndef LIBGOATVR_IMPL
typedef void goatvr_module;
typedef void goatvr_source;
#endif

enum goatvr_origin_mode { GOATVR_FLOOR, GOATVR_HEAD };
enum { GOATVR_LEFT, GOATVR_RIGHT };

enum goatvr_module_type {
	GOATVR_DISPLAY_MODULE,
	GOATVR_INPUT_MODULE
};

enum goatvr_user_gender {
	GOATVR_USER_UNKNOWN,
	GOATVR_USER_MALE,
	GOATVR_USE_FEMALE
};

enum {
	GOATVR_ACTION_GRAB,
	GOATVR_ACTION_POINT,
	GOATVR_ACTION_THUMB,
	GOATVR_ACTION_BIRD,
	GOATVR_ACTION_TRIGGER,
	GOATVR_ACTION_FIST,
	GOATVR_ACTION_NAV,

	GOATVR_NUM_ACTIONS
};

#define GOATVR_ALL_BUTTONS		0xffffffff

#ifdef __cplusplus
extern "C" {
#endif

int goatvr_init(void);
void goatvr_shutdown(void);

/* Detect which modules can be activated (which devices are available).
 * This is automatically called by goatvr_init, but can be called again
 * multiple times to re-detect devices, if necessary.
 */
void goatvr_detect(void);

/* when startvr is called all activated modules will start doing whatever the
 * fuck they are supposed to be doing in VR. If no explicit calls have been
 * made to activate specific modules, detected usable modules will be
 * activated based on internal arbitrary priorities.
 */
void goatvr_startvr(void);	/* enter virtual reality */
void goatvr_stopvr(void);	/* exit virtual reality */
int goatvr_invr(void);		/* are we in VR? */

/* GOATVR_FLOOR: the origin height is always at the user's floor level, and
 *  goatvr_recenter affects only the x/z components of the origin (default).
 * GOATVR_HEAD: the origin is at the users head, and is reset to the current
 *  head position every time goatvr_recenter is called.
 */
void goatvr_set_origin_mode(enum goatvr_origin_mode origin);
enum goatvr_origin_mode goatvr_get_origin_mode(void);

void goatvr_recenter(void);

/* returns true (non-zero) if the current configuration (active modules and
 * input overrides) supports head tracking, or false (zero) if the provided
 * view matrix just accounts for stereo separation.
 */
int goatvr_have_headtracking(void);
/* returns true if the active module provides hand tracking */
int goatvr_have_handtracking(void);
/* returns true if the hand tracking information is valid as of last update */
int goatvr_hand_active(int idx);

void goatvr_set_units_scale(float us);
float goatvr_get_units_scale(void);


/* ---- rendering ---- */
void goatvr_set_fb_size(int width, int height, float scale);
float goatvr_get_fb_scale(void);

/* framebuffer width and height (both viewports) */
int goatvr_get_fb_width(void);
int goatvr_get_fb_height(void);
/* width/height for each eye */
int goatvr_get_fb_eye_width(int eye);
int goatvr_get_fb_eye_height(int eye);
/* offset for each eye */
int goatvr_get_fb_eye_xoffset(int eye);
int goatvr_get_fb_eye_yoffset(int eye);

unsigned int goatvr_get_fb_texture(void);
/* fb_texture_width/fb_texture_height are for the whole texture (pow2) */
int goatvr_get_fb_texture_width(void);
int goatvr_get_fb_texture_height(void);

/* get the framebuffer object used as a VR render target. If an FBO wasn't
 * explicitly set with goatvr_set_fbo, then one is created automatically,
 * for the modules which need an FBO.
 */
unsigned int goatvr_get_fbo(void);

/* call glViewport for this eye */
void goatvr_viewport(int eye);

/* goatvr_view_matrix returns a view matrix (pointer to 16 floats in OpenGL
 * order) which can be used to render the scene from the viewpoint of each eye.
 * The matrix returned contains in part the inverse of goatvr_head_matrix,
 * concatenated with a translation for eye position relative to the head. */
float *goatvr_view_matrix(int eye);
/* return the projection matrix for each eye */
float *goatvr_projection_matrix(int eye, float znear, float zfar);

/* start drawing prepares for VR drawing, and binds FBO. */
void goatvr_draw_start(void);
 /* call before drawing each eye. calls glViewport internally */
void goatvr_draw_eye(int eye);
/* done drawing both eyes, the frame is ready to be presented */
void goatvr_draw_done(void);

/* some VR modules (only oculus_old currently) need to take over the user
 * window buffer swapping, and won't work properly if the user does a swap
 * at the end of every frame. This function returns true (non-zero) if the
 * user should perform buffer swaps on their window, or false (zero) if the
 * VR module needs to handle them.
 * example:
 *   if(goatvr_should_swap()) {
 *       SDL_GL_SwapWindow(win);
 *   }
 */
int goatvr_should_swap(void);

/* ---- tracking and input ---- */

/* valid if goatvr_have_headtracking() */
void goatvr_head_position(float *pos);
void goatvr_head_orientation(float *quat);
float *goatvr_head_matrix(void);

/* valid if goatvr_have_handtracking() and goatvr_hand_active(hand) */
void goatvr_hand_position(int hand, float *pos);
void goatvr_hand_orientation(int hand, float *quat);
float *goatvr_hand_matrix(int hand);

/* Check for high-level hand actions. The conditions under each of these is
 * triggered is module-specific. Also, multiple actions may be true at once */
int goatvr_action(int hand, int act);

/* buttons on input devices */
int goatvr_num_buttons(void);
const char *goatvr_button_name(int bn);
int goatvr_button_state(int bn);
int goatvr_lookup_button(const char *name);

/* analog axes on input devices. this doesn't count tracking axes,
 * only joysticks and analog triggers */
int goatvr_num_axes(void);
const char *goatvr_axis_name(int axis);
float goatvr_axis_value(int axis);
int goatvr_lookup_axis(const char *name);

/* pairs of axes might be grouped in 2D "sticks", and queried by calling
 * goatvr_stick_pos with the stick index, and a pointer to an array of two
 * floats. */
int goatvr_num_sticks(void);
const char *goatvr_stick_name(int stick);
void goatvr_stick_pos(int stick, float *pos);
int goatvr_lookup_stick(const char *name);

/* ---- module management ---- */
/* Multiple input modules can be activated at the same time, but only one
 * display module. Activating one display module, implicitly deactivates the
 * previously activated one.
 */
int goatvr_activate_module(goatvr_module *vrmod);
int goatvr_deactivate_module(goatvr_module *vrmod);

int goatvr_num_modules(void);
goatvr_module *goatvr_get_module(int idx);
goatvr_module *goatvr_find_module(const char *name);

/* retrieve information about a module */
const char *goatvr_module_name(goatvr_module *mod);
enum goatvr_module_type goatvr_module_type(goatvr_module *mod);
int goatvr_module_active(goatvr_module *mod);
int goatvr_module_usable(goatvr_module *mod);

int goatvr_module_headtracking(goatvr_module *mod);
int goatvr_module_handtracking(goatvr_module *mod);
int goatvr_module_num_buttons(goatvr_module *mod);
int goatvr_module_num_axes(goatvr_module *mod);
int goatvr_module_num_sticks(goatvr_module *mod);
const char *goatvr_module_button_name(goatvr_module *mod, int bn);
const char *goatvr_module_axis_name(goatvr_module *mod, int axis);
const char *goatvr_module_stick_name(goatvr_module *mod, int stick);

/* ---- user information ---- */
float goatvr_get_eye_height(void);
enum goatvr_user_gender goatvr_get_user_gender(void);

/* ---- utility functions ---- */
void goatvr_util_quat_to_matrix(float *mat, const float *quat);
/* invert a matrix. returns 0 on success, -1 if singular */
int goatvr_util_invert_matrix(float *inv, const float *mat);

#ifdef __cplusplus
}
#endif

#endif	/* LIBGOATVR_H_ */
