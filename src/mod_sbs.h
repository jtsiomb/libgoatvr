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
#ifndef MOD_SBS_H_
#define MOD_SBS_H_

#include "module.h"

namespace goatvr {

class ModuleSBS : public Module {
protected:
	bool started;
	int win_width, win_height;

	float ipd;

	goatvr_origin_mode origin_mode;

public:
	ModuleSBS();
	~ModuleSBS();

	bool init();
	void destroy();

	enum goatvr_module_type get_type() const;
	const char *get_name() const;

	bool detect();
	bool start();

	void set_origin_mode(goatvr_origin_mode mode);

	void set_fbsize(int width, int height, float fbscale);

	void get_view_matrix(Mat4 &mat, int eye) const;
	void get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const;
};

}	// namespace goatvr

#endif	// MOD_SBS_H_
