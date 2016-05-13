#ifndef DEVICE_H_
#define DEVICE_H_

#include <gmath/gmath.h>
#include "module.h"

namespace goatvr {

class Device {
	Module *module;	/* handled by module */
	void *mod_dev;	/* module-internal device handle */

	char *name;
	int num_buttons, num_axes;
	int is_spatial;

	gph::Vec3 pos;
	gph::Quat rot;
	gph::Mat4 matrix;
};

}

#endif	/* DEVICE_H_ */
