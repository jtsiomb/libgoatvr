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
#ifndef GOATVR_IMPL_H_
#define GOATVR_IMPL_H_

#include <vector>
#include <gmath/gmath.h>

namespace goatvr {

class Module;

struct Source {
	Module *mod;
	void *mod_data;

	Mat4 xform;	 // matrix for the spatial sources
};

}

typedef goatvr::Module goatvr_module;
typedef goatvr::Source goatvr_source;

#define LIBGOATVR_IMPL
#include "goatvr.h"

namespace goatvr {

void set_user_eye_height(float height);
void set_user_gender(goatvr_user_gender gender);

void add_source(Source *s);
void remove_source(Source *s);

unsigned int next_pow2(unsigned int x);

void calc_matrix(Mat4 &mat, const Vec3 &pos, const Quat &rot);
void calc_inv_matrix(Mat4 &mat, const Vec3 &pos, const Quat &rot);

}

#ifdef _MSC_VER
#define strcasecmp	stricmp
#endif

#endif	/* GOATVR_IMPL_H_ */
