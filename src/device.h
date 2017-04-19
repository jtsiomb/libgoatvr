/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2016  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef DEVICE_H_
#define DEVICE_H_

#include <gmath/gmath.h>
#include "module.h"

// TODO

namespace goatvr {

class Source {
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
