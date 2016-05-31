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
#include "opengl.h"
#include "mod_stereo.h"

REG_MODULE(stereo, ModuleStereo)

using namespace goatvr;

ModuleStereo::ModuleStereo()
{
}

ModuleStereo::~ModuleStereo()
{
	destroy();
}

const char *ModuleStereo::get_name() const
{
	return "stereo";
}

bool ModuleStereo::detect()
{
	GLboolean val;
	glGetBooleanv(GL_STEREO, &val);
	return val == GL_TRUE;
}

void ModuleStereo::draw_start()
{
	// first select drawing to both back buffers to allow the user to clear
	// them at the same time.
	glDrawBuffer(GL_BACK);
}

void ModuleStereo::draw_eye(int eye)
{
	glDrawBuffer(eye == GOATVR_LEFT ? GL_BACK_LEFT : GL_BACK_RIGHT);
}

void ModuleStereo::draw_done()
{
	glDrawBuffer(GL_BACK);	// reset to both buffers again
}
