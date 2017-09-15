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
#include "opengl.h"
#include "mod_sbs.h"

REG_MODULE(sbs, ModuleSBS)

using namespace goatvr;

ModuleSBS::ModuleSBS()
{
	win_width = win_height = -1;
	ipd = 0.064f;		// default IPD 6.4cm
}

ModuleSBS::~ModuleSBS()
{
	destroy();
}

bool ModuleSBS::init()
{
	if(!Module::init()) {
		return false;
	}
	return true;
}

void ModuleSBS::destroy()
{
	stop();
	Module::destroy();
}

enum goatvr_module_type ModuleSBS::get_type() const
{
	return GOATVR_DISPLAY_MODULE;
}

const char *ModuleSBS::get_name() const
{
	return "sbs";
}

bool ModuleSBS::detect()
{
	avail = true;
	return true;
}

bool ModuleSBS::start()
{
	if(win_width == -1) {
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		win_width = vp[2] + vp[0];
		win_height = vp[3] + vp[1];
	}
	return true;
}

void ModuleSBS::set_origin_mode(goatvr_origin_mode mode)
{
	origin_mode = mode;
}

void ModuleSBS::set_fbsize(int width, int height, float fbscale)
{
	win_width = width;
	win_height = height;
}

void ModuleSBS::get_view_matrix(Mat4 &mat, int eye) const
{
	float eye_offs[] = {0.5f * ipd, -0.5f * ipd};
	float units_scale = goatvr_get_units_scale();

	Module::get_view_matrix(mat, eye);
	mat.translate(eye_offs[eye] * units_scale, 0, 0);

	if(origin_mode == GOATVR_FLOOR) {
		mat.translate(0, -1.65 * units_scale, 0);
	}
}

void ModuleSBS::get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const
{
	// TODO let the user set the fov
	float vfov = deg_to_rad(60);
	float aspect = (float)win_width / (float)win_height;
	float top = znear * tan(vfov * 0.5);
	float right = top * aspect;

	static const float offs[] = {1.0, -1.0};
	float shift = offs[eye] * (ipd * 0.5 * znear /* / focus_dist? */);

	mat.frustum(-right + shift, right + shift, -top, top, znear, zfar);
}
