#ifndef VR_H_
#define VR_H_

/* unit: pixels */
#define VR_OPT_DISPLAY_WIDTH	"display-xres"
#define VR_OPT_DISPLAY_HEIGHT	"display-yres"
#define VR_OPT_LEYE_XRES	"left-eye-xres"
#define VR_OPT_LEYE_YRES	"left-eye-yres"
#define VR_OPT_REYE_XRES	"right-eye-xres"
#define VR_OPT_REYE_YRES	"right-eye-yres"
#define VR_OPT_WIN_XOFFS	"win-xoffset"
#define VR_OPT_WIN_YOFFS	"win-yoffset"
/* unit: meters */
#define VR_OPT_EYE_HEIGHT	"eye-height"
#define VR_OPT_IPD			"ipd"

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

void vr_set_opti(const char *optname, int val);
void vr_set_optf(const char *optname, float val);
int vr_get_opti(const char *optname);
float vr_get_optf(const char *optname);

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
