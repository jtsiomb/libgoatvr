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
#include "mod_anaglyph.h"

REG_MODULE(anaglyph, ModuleAnaglyph)

using namespace goatvr;

ModuleAnaglyph::ModuleAnaglyph()
{
	first_eye = true;
}

ModuleAnaglyph::~ModuleAnaglyph()
{
	destroy();
}

const char *ModuleAnaglyph::get_name() const
{
	return "anaglyph";
}

void ModuleAnaglyph::draw_start()
{
	first_eye = true;
}

void ModuleAnaglyph::draw_eye(int eye)
{
	if(first_eye) {
		first_eye = false;
	} else {
		// for each subsequent eye, clear the depth buffer
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	if(eye == GOATVR_LEFT) {
		glColorMask(1, 0, 0, 1);
	} else {
		glColorMask(0, 1, 1, 1);
	}

	glViewport(0, 0, win_width, win_height);
}

void ModuleAnaglyph::draw_done()
{
	glColorMask(1, 1, 1, 1);
}
