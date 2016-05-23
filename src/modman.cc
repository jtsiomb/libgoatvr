#include <string.h>
#include <vector>
#include <set>
#include <algorithm>
#include "modman.h"

static struct {
	const char *name;
	int prio;
} modprio[] = {
	{ "oculus", 128 },
	{ "openvr", 127 },
	{ "glstereo", 64 },
	{ "anaglyph", 63},
	{ "sbs", 62},
	{0, 0}
};

namespace goatvr {

Module *render_module;

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
	render_module = 0;
}

void add_module(Module *m)
{
	if(std::find(modules.begin(), modules.end(), m) == modules.end()) {
		if(!m->init()) {
			return;
		}
		modules.push_back(m);

		if(m->get_type() == MODULE_RENDERING) {
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
	// only allow a single active rendering module
	if(render_module && m->get_type() == MODULE_RENDERING) {
		deactivate(render_module);
	}
	active.insert(m);
	render_module = m;
}

void deactivate(Module *m)
{
	if(m->get_type() == MODULE_RENDERING) {
		render_module = 0;
	}
	active.erase(m);
}

void start()
{
	for(Module *m : active) {
		m->start();
	}
}

void stop()
{
	for(Module *m : active) {
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
	if(render_module) {
		render_module->draw_start();
	}
}

void draw_eye(int eye)
{
	if(render_module) {
		render_module->draw_eye(eye);
	}
}

void draw_done()
{
	if(render_module) {
		render_module->draw_done();
	}
}

}	// namespace goatvr
