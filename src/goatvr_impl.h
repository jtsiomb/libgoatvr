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

namespace goatvr {

class Module;
class Source;

}

typedef goatvr::Module goatvr_module;
typedef goatvr::Source goatvr_source;

#define LIBGOATVR_IMPL
#include "goatvr.h"

namespace goatvr {

void set_user_eye_height(float height);
void set_user_gender(goatvr_user_gender gender);

unsigned int next_pow2(unsigned int x);

}

#endif	/* GOATVR_IMPL_H_ */
