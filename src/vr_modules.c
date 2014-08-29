/* XXX this might become partly auto-generated in the future */
#include <stdio.h>
#include <stdlib.h>
#include "vr_impl.h"

struct vr_module *vr_module_libovr(void);
struct vr_module *vr_module_openhmd(void);
struct vr_module *vr_module_null(void);

static struct vr_module *modules;
static int num_modules, modules_max_size;

static int *active_modules;
static int num_act_modules, act_modules_max_size;


void vr_init_modules(void)
{
	struct vr_module *m;

	vr_clear_modules();

	if((m = vr_module_libovr())) {
		vr_register_module(m);
	}

	if((m = vr_module_openhmd())) {
		vr_register_module(m);
	}

	if((m = vr_module_null())) {
		vr_register_module(m);
	}
}

void vr_clear_modules(void)
{
	free(modules);
	free(active_modules);
	modules = 0;
	num_modules = modules_max_size = 0;
	active_modules = 0;
	num_act_modules = act_modules_max_size = 0;
}

void vr_register_module(struct vr_module *mod)
{
	if(num_modules >= modules_max_size) {
		int newsz = modules_max_size ? modules_max_size * 2 : 2;
		struct vr_module *newmods = realloc(modules, newsz * sizeof *newmods);
		if(!newmods) {
			fprintf(stderr, "failed to resize modules array up to %d\n", newsz);
			return;
		}
		modules = newmods;
		modules_max_size = newsz;
	}
	modules[num_modules++] = *mod;
}

int vr_get_num_modules(void)
{
	return num_modules;
}

struct vr_module *vr_get_module(int idx)
{
	return modules + idx;
}

void vr_activate_module(int idx)
{
	if(num_act_modules >= act_modules_max_size) {
		int newsz = act_modules_max_size ? act_modules_max_size * 2 : 2;
		int *newact = realloc(active_modules, newsz * sizeof *newact);
		if(!newact) {
			fprintf(stderr, "failed to resize active modules array up to %d\n", newsz);
			return;
		}
		active_modules = newact;
		act_modules_max_size = newsz;
	}
	active_modules[num_act_modules++] = idx;
}

int vr_get_num_active_modules(void)
{
	return num_act_modules;
}

struct vr_module *vr_get_active_module(int idx)
{
	return modules + active_modules[idx];
}
