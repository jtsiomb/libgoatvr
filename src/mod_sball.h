/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2017  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef MOD_SBALL_H_
#define MOD_SBALL_H_

#include "module.h"

namespace goatvr {

class ModuleSpaceball : public Module {
protected:
	int fd;
	unsigned int bnstate;
	float axis[6];
	Vec3 pos;
	Quat rot;
	Mat4 xform;

public:
	ModuleSpaceball();
	~ModuleSpaceball();

	bool init();
	void destroy();

	enum goatvr_module_type get_type() const;
	const char *get_name() const;

	bool detect();

	void start();
	void stop();

	void update();

	int num_buttons() const;
	const char *get_button_name(int bn) const;
	unsigned int get_button_state(unsigned int mask) const;

	int num_axes() const;
	const char *get_axis_name(int axis) const;
	float get_axis_value(int axis) const;
};

}	// namespace goatvr

#endif	// MOD_SBALL_H_
