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
#ifndef SOURCE_H_
#define SOURCE_H_

#include <gmath/gmath.h>
#include "module.h"

namespace goatvr {

class Source {
protected:
	Module *module;	/* handled by module */

public:
	explicit Source(Module *mod = 0);
	virtual ~Source();

	virtual const char *get_name() const;
	virtual int get_num_buttons() const;
	virtual int get_num_axes() const;
	virtual bool is_spatial() const;

	virtual Vec3 get_position() const;
	virtual Quat get_rotation() const;
	virtual Mat4 get_matrix() const;
};

}

#endif	/* SOURCE_H_ */
