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
#ifndef MOD_ANAGLYPH_H_
#define MOD_ANAGLYPH_H_

#include "mod_sbs.h"

namespace goatvr {

class ModuleAnaglyph : public ModuleSBS {
protected:
	bool first_eye;	// true just after draw_start, false after draw_eye returns

public:
	ModuleAnaglyph();
	~ModuleAnaglyph();

	const char *get_name() const;

	void draw_start();
	void draw_eye(int eye);
	void draw_done();
};

}	// namespace goatvr

#endif	// MOD_ANAGLYPH_H_
