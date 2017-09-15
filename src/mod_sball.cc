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
#ifdef USE_MOD_SBALL

#include <vector>
#include <spnav.h>
#include "mod_sball.h"
#include "goatvr_impl.h"

// TODO find a way to get the actual number of buttons
#define NUM_BUTTONS	16
static const char *button_names[] = {
	"spaceball button 00", "spaceball button 01", "spaceball button 02", "spaceball button 03",
	"spaceball button 04", "spaceball button 05", "spaceball button 06", "spaceball button 07",
	"spaceball button 08", "spaceball button 09", "spaceball button 10", "spaceball button 11",
	"spaceball button 12", "spaceball button 13", "spaceball button 14", "spaceball button 15"
};
static const char *axis_names[] = {
	"spaceball tx", "spaceball ty", "spaceball tz",
	"spaceball rx", "spaceball ry", "spaceball rz"
};

REG_MODULE(sball, ModuleSpaceball)

using namespace goatvr;

ModuleSpaceball::ModuleSpaceball()
{
	fd = -1;
}

ModuleSpaceball::~ModuleSpaceball()
{
	destroy();
}

bool ModuleSpaceball::init()
{
	if(!Module::init()) {
		return false;
	}
	return true;
}

void ModuleSpaceball::destroy()
{
	Module::destroy();
}

enum goatvr_module_type ModuleSpaceball::get_type() const
{
	return GOATVR_INPUT_MODULE;
}

const char *ModuleSpaceball::get_name() const
{
	return "spaceball";
}

bool ModuleSpaceball::detect()
{
	int fd = spnav_open();
	if(fd >= 0) {
		spnav_close();
		avail = true;
		return true;
	}
	avail = false;
	return false;
}

bool ModuleSpaceball::start()
{
	if(fd >= 0) return true;

	if((fd = spnav_open()) < 0) {
		print_error("failed to open connection to the spacenav daemon\n");
		return false;
	}
	return true;
}

void ModuleSpaceball::stop()
{
	if(fd < 0) return;

	spnav_close();
	fd = -1;
}

void ModuleSpaceball::update()
{
	if(fd < 0) return;

	bool xform_dirty = false;
	spnav_event ev;

	while(spnav_poll_event(&ev)) {
		if(ev.type == SPNAV_EVENT_MOTION) {
			xform_dirty = true;
			axis[0] = ev.motion.x;
			axis[1] = ev.motion.y;
			axis[2] = ev.motion.z;
			axis[3] = ev.motion.rx;
			axis[4] = ev.motion.ry;
			axis[5] = ev.motion.rz;

			if(ev.motion.rx || ev.motion.ry || ev.motion.rz) {
				Vec3 rvec = Vec3(ev.motion.rx, ev.motion.ry, ev.motion.rz);
				float len = length(rvec);
				Vec3 axis = Vec3(rvec.x / len, rvec.y / len, -rvec.z / len);
				rot.rotate(axis, len * 0.001);
			}

			Vec3 dir = Vec3(ev.motion.x * 0.001, ev.motion.y * 0.001, -ev.motion.z * 0.001);
			//sdata->pos += rotate(dir, sdata->rot);
			pos += dir;

		} else {
			if(ev.button.press) {
				bnstate |= 1 << ev.button.bnum;
			} else {
				bnstate &= ~(1 << ev.button.bnum);
			}
		}

		spnav_remove_events(SPNAV_EVENT_MOTION);
	}

	if(xform_dirty) {
		calc_matrix(xform, pos, rot);
	}
}


int ModuleSpaceball::num_buttons() const
{
	return NUM_BUTTONS;
}

const char *ModuleSpaceball::get_button_name(int bn) const
{
	return button_names[bn];
}

unsigned int ModuleSpaceball::get_button_state(unsigned int mask) const
{
	return bnstate & mask;
}

int ModuleSpaceball::num_axes() const
{
	return 6;
}

const char *ModuleSpaceball::get_axis_name(int axis) const
{
	return axis_names[axis];
}

float ModuleSpaceball::get_axis_value(int axis) const
{
	return this->axis[axis];
}

#else

#include "module.h"
// this expands to an empty register_mod_sball() function
NOREG_MODULE(sball)

#endif	// USE_MOD_SBALL
