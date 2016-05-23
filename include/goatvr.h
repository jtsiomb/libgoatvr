#ifndef LIBGOATVR_H_
#define LIBGOATVR_H_

#ifndef LIBGOATVR_IMPL
typedef void goatvr_module;
typedef void goatvr_device;
#endif

enum goatvr_origin_mode { GOATVR_FLOOR, GOATVR_HEAD };

enum { GOATVR_LEFT, GOATVR_RIGHT };

enum {
	GOATVR_DEV_HMD,
	GOATVR_DEV_LEFT_HAND,
	GOATVR_DEV_RIGHT_HAND
};

enum goatvr_user_gender { GOATVR_USER_UNKNOWN, GOATVR_USER_MALE, GOATVR_USE_FEMALE };

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
 * explicitly set with goatvr_set_fbo, then one is created automatically */
unsigned int goatvr_get_fbo(void);

/* call glViewport for this eye */
void goatvr_viewport(int eye);

float *goatvr_view_matrix(int eye);
float *goatvr_projection_matrix(int eye, float znear, float zfar);

/* start drawing prepares for VR drawing, and binds FBO. */
void goatvr_draw_start(void);
 /* call before drawing each eye. calls glViewport internally */
void goatvr_draw_eye(int eye);
/* done drawing both eyes, the frame is ready to be presented */
void goatvr_draw_done(void);

/* ---- input device handling ---- */
int goatvr_num_devices(void);
goatvr_device *goatvr_get_device(int idx);

const char *goatvr_device_name(goatvr_device *dev);
int goatvr_device_spatial(goatvr_device *dev);
int goatvr_device_num_axes(goatvr_device *dev);
int goatvr_device_num_buttons(goatvr_device *dev);

/* get the device position. expects an array of 3 floats */
void goatvr_device_position(goatvr_device *dev, float *pos);
/* get the device orientation. expects an array of 4 floats (quaternion xyzw) */
void goatvr_device_orientation(goatvr_device *dev, float *quat);
/* get the device transformation matrix (returns pointer to 16 floats opengl-order) */
float *goatvr_device_matrix(goatvr_device *dev);


/* ---- module management ---- */
/* Multiple modules can be activated at the same time, but some might be
 * mutually exclusive (e.g. oculus and openvr).
 */
int goatvr_activate_module(goatvr_module *vrmod);
int goatvr_deactivate_module(goatvr_module *vrmod);

int goatvr_num_modules(void);
goatvr_module *goatvr_get_module(int idx);
goatvr_module *goatvr_find_module(const char *name);

const char *goatvr_module_name(goatvr_module *vrmod);
int goatvr_module_active(goatvr_module *vrmod);
int goatvr_module_usable(goatvr_module *vrmod);

/* ---- information retrieval ---- */
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
