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
#include <stdio.h>
#include <stdarg.h>
#include "module.h"
#include "modman.h"

using namespace goatvr;

Module::Module()
{
	avail = act = false;
	prio = 0;

	for(int i=0; i<3; i++) {
		track_src[i] = def_track_src[i] = 0;
	}
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
	for(size_t i=0; i<inp_sources.size(); i++) {
		delete inp_sources[i];
	}
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

void Module::start()
{
}

void Module::stop()
{
}

void Module::set_origin_mode(goatvr_origin_mode mode)
{
}

void Module::recenter()
{
}

bool Module::have_head_tracking() const
{
	return get_head_source() != 0;
}

bool Module::have_hand_tracking() const
{
	return get_hand_source(0) != 0 || get_hand_source(1) != 0;
}

int Module::num_input_sources() const
{
	return (int)inp_sources.size();
}

Source *Module::get_input_source(int idx) const
{
	return inp_sources[idx];
}

void Module::set_default_sources()
{
	for(int i=0; i<3; i++) {
		track_src[i] = def_track_src[i];
	}
}

void Module::set_head_source(Source *src)
{
	track_src[0] = src;
}

Source *Module::get_head_source() const
{
	return track_src[0];
}

void Module::set_hand_source(int idx, Source *src)
{
	track_src[idx + 1] = src;
}

Source *Module::get_hand_source(int idx) const
{
	return track_src[idx - 1];
}

const char *Module::get_soure_name(void *sdata) const
{
	return "unknown";
}

bool Module::is_source_spatial(void *sdata) const
{
	return false;
}

int Module::get_source_num_axes(void *sdata) const
{
	return 0;
}

int Module::get_source_num_buttons(void *sdata) const
{
	return 0;
}

Vec3 Module::get_source_pos(void *sdata) const
{
	return Vec3(0, 0, 0);
}

Quat Module::get_source_rot(void *sdata) const
{
	return Quat::identity;
}

void Module::update()
{
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

Mat4 Module::get_view_matrix(int eye) const
{
	return Mat4::identity;
}

Mat4 Module::get_proj_matrix(int eye, float znear, float zfar) const
{
	return Mat4::identity;
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
