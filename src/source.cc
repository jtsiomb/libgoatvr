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
#include "source.h"

using namespace goatvr;

Source::Source(Module *mod)
{
	module = mod;
}

Source::~Source()
{
}

const char *Source::get_name() const
{
	return "unknown input source";
}

int Source::get_num_buttons() const
{
	return 0;
}

int Source::get_num_axes() const
{
	return 0;
}

bool Source::is_spatial() const
{
	return false;
}

Vec3 Source::get_position() const
{
	return Vec3(0, 0, 0);
}

Quat Source::get_rotation() const
{
	return Quat(0, 0, 0, 1);
}

Mat4 Source::get_matrix() const
{
	return Mat4::identity;
}