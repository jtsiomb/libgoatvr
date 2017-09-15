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
#include <stdio.h>
#include <stdarg.h>
#include "module.h"
#include "modman.h"

using namespace goatvr;

Module::Module()
{
	avail = act = false;
	prio = 0;
}

Module::~Module()
{
}

bool Module::init()
{
	return true;
}

void Module::destroy()
{
}

void Module::set_priority(int p)
{
	prio = p;
}

int Module::get_priority() const
{
	return prio;
}

bool Module::detect()
{
	return false;
}

bool Module::usable() const
{
	return avail;
}

void Module::activate()
{
	act = true;
}

void Module::deactivate()
{
	act = false;
}

bool Module::active() const
{
	return act;
}

bool Module::start()
{
	return true;
}

void Module::stop()
{
}

void Module::update()
{
}

void Module::set_origin_mode(goatvr_origin_mode mode)
{
}

void Module::recenter()
{
}

bool Module::have_headtracking() const
{
	return false;
}

bool Module::have_handtracking() const
{
	return false;
}

bool Module::hand_active(int idx) const
{
	return false;
}

int Module::num_buttons() const
{
	return 0;
}

const char *Module::get_button_name(int bn) const
{
	return 0;
}

unsigned int Module::get_button_state(unsigned int mask) const
{
	return 0;
}

int Module::num_axes() const
{
	return 0;
}

const char *Module::get_axis_name(int axis) const
{
	return 0;
}

float Module::get_axis_value(int axis) const
{
	return 0.0f;
}

int Module::num_sticks() const
{
	return 0;
}

const char *Module::get_stick_name(int stick) const
{
	return 0;
}

Vec2 Module::get_stick_pos(int stick) const
{
	return Vec2(0, 0);
}

void Module::set_fbsize(int width, int height, float fbscale)
{
}

RenderTexture *Module::get_render_texture()
{
	return 0;
}

void Module::draw_start()
{
}

void Module::draw_eye(int eye)
{
}

void Module::draw_done()
{
}

void Module::draw_mirror()
{
}

bool Module::should_swap() const
{
	return true;
}

void Module::get_view_matrix(Mat4 &mat, int eye) const
{
	mat = Mat4::identity;
}

void Module::get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const
{
	mat = Mat4::identity;
}

Vec3 Module::get_head_position() const
{
	return Vec3(0, 0, 0);
}

Quat Module::get_head_orientation() const
{
	return Quat::identity;
}

void Module::get_head_matrix(Mat4 &mat) const
{
	mat = Mat4::identity;
}

Vec3 Module::get_hand_position(int hand) const
{
	return Vec3(0, 0, 0);
}

Quat Module::get_hand_orientation(int hand) const
{
	return Quat::identity;
}

void Module::get_hand_matrix(Mat4 &mat, int hand) const
{
	mat = Mat4::identity;
}

void Module::print_info(const char *fmt, ...) const
{
	va_list ap;

	printf("module %s: ", get_name());

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void Module::print_error(const char *fmt, ...) const
{
	va_list ap;

	fprintf(stderr, "module %s: ", get_name());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
