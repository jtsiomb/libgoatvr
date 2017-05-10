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

#include <spnav.h>
#include "mod_sball.h"
#include "goatvr_impl.h"

struct SrcData {
	Vec3 pos;
	Quat rot;
};

REG_MODULE(sball, ModuleSpaceball)

using namespace goatvr;

ModuleSpaceball::ModuleSpaceball()
{
	fd = -1;
	src = 0;
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

ModuleType ModuleSpaceball::get_type() const
{
	return MODULE_OTHER;
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

void ModuleSpaceball::start()
{
	if(fd >= 0) return;

	if((fd = spnav_open()) < 0) {
		print_error("failed to open connection to the spacenav daemon\n");
		return;
	}

	SrcData *sd = new SrcData;

	src = new Source;
	src->mod = this;
	src->mod_data = sd;
	add_source(src);
}

void ModuleSpaceball::stop()
{
	if(fd < 0) return;

	if(src) {
		remove_source(src);
		delete (SrcData*)src->mod_data;
		delete src;
		src = 0;
	}

	spnav_close();
	fd = -1;
}

const char *ModuleSpaceball::get_source_name(void *sdata) const
{
	return "spaceball";
}

bool ModuleSpaceball::is_source_spatial(void *sdata) const
{
	return true;
}

Vec3 ModuleSpaceball::get_source_pos(void *sdata) const
{
	return ((SrcData*)sdata)->pos;
}

Quat ModuleSpaceball::get_source_rot(void *sdata) const
{
	return ((SrcData*)sdata)->rot;
}

void ModuleSpaceball::update()
{
	SrcData *sdata = (SrcData*)src->mod_data;
	spnav_event ev;

	while(spnav_poll_event(&ev)) {
		if(ev.type == SPNAV_EVENT_MOTION) {
			if(ev.motion.rx || ev.motion.ry || ev.motion.rz) {
				Vec3 rvec = Vec3(ev.motion.rx, ev.motion.ry, ev.motion.rz);
				float len = length(rvec);
				Vec3 axis = Vec3(rvec.x / len, rvec.y / len, -rvec.z / len);
				sdata->rot.rotate(axis, len * 0.001);
			}

			Vec3 dir = Vec3(ev.motion.x * 0.001, ev.motion.y * 0.001, -ev.motion.z * 0.001);
			//sdata->pos += rotate(dir, sdata->rot);
			sdata->pos += dir;

		} else {
			if(ev.button.press) {
				// TODO
			}
		}

		spnav_remove_events(SPNAV_EVENT_MOTION);
	}

	calc_matrix(src->xform, sdata->pos, sdata->rot);
}

#else

#include "module.h"
// this expands to an empty register_mod_sball() function
NOREG_MODULE(sball)

#endif	// USE_MOD_SBALL
