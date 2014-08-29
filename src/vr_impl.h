#ifndef VR_IMPL_H_
#define VR_IMPL_H_

#include "vr.h"
#include "opt.h"

struct vr_module {
	char *name;

	int (*init)(void);
	void (*cleanup)(void);

	int (*set_option)(const char *opt, enum opt_type type, void *valp);
	int (*get_option)(const char *opt, enum opt_type type, void *valp);

	void (*translation)(int eye, float *vec);
	void (*rotation)(int eye, float *quat);

	void (*view_matrix)(int eye, float *mat);
	void (*proj_matrix)(int eye, float znear, float zfar, float *mat);

	void (*begin)(int eye);
	void (*end)(void);
	int (*present)(void);

	void (*set_eye_texture)(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax);

	void (*recenter)(void);
};

void vr_init_modules(void);

void vr_clear_modules(void);
void vr_register_module(struct vr_module *mod);

int vr_get_num_modules(void);
struct vr_module *vr_get_module(int idx);

void vr_activate_module(int idx);

int vr_get_num_active_modules(void);
struct vr_module *vr_get_active_module(int idx);

#endif	/* VR_IMPL_H_ */
