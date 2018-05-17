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
#include <string.h>
#include <vector>
#include <set>
#include <algorithm>
#include "modman.h"
#include "inpman.h"

static struct {
	const char *name;
	int prio;
} modprio[] = {
	{ "oculus", 128 },
	{ "openvr", 127 },
	{ "oculus_old", 126 },
	{ "openhmd", 125 },
	{ "stereo", 64 },
	{ "anaglyph", 63},
	{ "sbs", 62},
	{0, 0}
};

namespace goatvr {

Module *display_module;

static std::vector<Module*> modules;
static std::set<Module*> active;
static int num_avail;

void destroy_modules()
{
	for(size_t i=0; i<modules.size(); i++) {
		delete modules[i];
	}
	modules.clear();
	active.clear();
	num_avail = 0;
	display_module = 0;
}

void add_module(Module *m)
{
	if(std::find(modules.begin(), modules.end(), m) == modules.end()) {
		if(!m->init()) {
			return;
		}
		modules.push_back(m);

		if(m->get_type() == GOATVR_DISPLAY_MODULE) {
			// assign a priority
			for(int i=0; modprio[i].name; i++) {
				if(strcmp(m->get_name(), modprio[i].name) == 0) {
					m->set_priority(modprio[i].prio);
				}
			}
		}
	}
}

int get_num_modules()
{
	return (int)modules.size();
}

Module *get_module(int idx)
{
	return modules[idx];
}

Module *find_module(const char *name)
{
	for(size_t i=0; i<modules.size(); i++) {
		if(strcmp(modules[i]->get_name(), name) == 0) {
			return modules[i];
		}
	}
	return 0;
}

int get_num_usable_modules()
{
	return num_avail;
}

void detect()
{
	num_avail = 0;
	for(size_t i=0; i<modules.size(); i++) {
		if(modules[i]->detect()) {
			++num_avail;
		}
	}
}

void activate(Module *m)
{
	if(m->get_type() == GOATVR_DISPLAY_MODULE) {
		// only allow a single active rendering module
		if(display_module) {
			deactivate(display_module);
		}
		display_module = m;
	}
	active.insert(m);
}

void deactivate(Module *m)
{
	if(m->get_type() == GOATVR_DISPLAY_MODULE) {
		display_module = 0;
	}
	active.erase(m);
}

bool start()
{
	for(Module *m : active) {
		if(!m->start() && m->get_type() == GOATVR_DISPLAY_MODULE) {
			display_module = 0;	// TODO fallback to the next available display module?
			return false;
		}
		inp_add_module(m);
	}
	return true;
}

void stop()
{
	for(Module *m : active) {
		inp_remove_module(m);
		m->stop();
	}
}

void update()
{
	for(Module *m : active) {
		m->update();
	}
}

void draw_start()
{
	if(display_module) {
		display_module->draw_start();
	}
}

void draw_eye(int eye)
{
	if(display_module) {
		display_module->draw_eye(eye);
	}
}

void draw_done()
{
	if(display_module) {
		display_module->draw_done();
	}
}

}	// namespace goatvr
