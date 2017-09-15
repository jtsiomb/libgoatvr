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
#ifndef MODMAN_H_
#define MODMAN_H_

#include <vector>
#include "module.h"

namespace goatvr {

extern Module *display_module;

void destroy_modules();

void add_module(Module *m);
int get_num_modules();
Module *get_module(int idx);
Module *find_module(const char *name);
int get_num_usable_modules();

// detect is performed on all modules to figure out which are usable
void detect();

void activate(Module *m);
void deactivate(Module *m);

// vr operations to be performed on all active modules
bool start();
void stop();
void update();

// operations to be performed on the active rendering module
void draw_start();
void draw_eye(int eye);
void draw_done();

}	// namespace goatvr

#endif	// MODMAN_H_
