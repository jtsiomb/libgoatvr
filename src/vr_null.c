#include "vr_impl.h"

static int init(void)
{
	return 0;
}

struct vr_module *vr_module_null(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "null";
		m.init = init;
	}
	return &m;
}
