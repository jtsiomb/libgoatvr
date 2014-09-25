#ifndef VR_H_
#define VR_H_

/* unit: pixels */
#define VR_DISPLAY_WIDTH	"display-xres"
#define VR_DISPLAY_HEIGHT	"display-yres"
#define VR_LEYE_XRES		"left-eye-xres"
#define VR_LEYE_YRES		"left-eye-yres"
#define VR_REYE_XRES		"right-eye-xres"
#define VR_REYE_YRES		"right-eye-yres"
#define VR_WIN_XOFFS		"win-xoffset"
#define VR_WIN_YOFFS		"win-yoffset"
#define VR_EYE_RES_SCALE	"eye-res-scale"		/* default 1 */
/* unit: meters */
#define VR_EYE_HEIGHT		"eye-height"
#define VR_IPD				"ipd"

enum {
	VR_EYE_LEFT,
	VR_EYE_RIGHT
};

#ifdef __cplusplus
extern "C" {
#endif

int vr_init(void);
void vr_shutdown(void);

int vr_module_count(void);
const char *vr_module_name(int idx);

int vr_use_module(int idx);
int vr_use_module_named(const char *name);

void vr_seti(const char *optname, int val);
void vr_setf(const char *optname, float val);
int vr_geti(const char *optname);
float vr_getf(const char *optname);
/* variants of the get functions, with an additional "default value"
 * argument, which is returned if the requested option is missing
 */
int vr_geti_def(const char *optname, int def_val);
float vr_getf_def(const char *optname, float def_val);

int vr_view_translation(int eye, float *vec);
int vr_view_rotation(int eye, float *quat);

/* returns non-zero if the active vr module provides this kind of matrix
 * information, otherwise it returns zero, and sets mat to identity
 */
int vr_view_matrix(int eye, float *mat);
int vr_proj_matrix(int eye, float znear, float zfar, float *mat);

void vr_begin(int eye);
void vr_end(void);
int vr_swap_buffers(void);

/* set the output texture or separate textures for each eye */
void vr_output_texture(unsigned int tex, float umin, float vmin, float umax, float vmax);
void vr_output_texture_eye(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax);

void vr_recenter(void);

#ifdef __cplusplus
}
#endif

#endif	/* VR_H_ */
