#include <string.h>
#include <vector>
#include <set>
#include <algorithm>
#include "modman.h"

namespace goatvr {

Module *render_module;

static std::vector<Module*> modules;
static std::set<Module*> active;
static int num_avail;

void add_module(Module *m)
{
	if(std::find(modules.begin(), modules.end(), m) == modules.end()) {
		modules.push_back(m);
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
		render_module = m;
	}
	active.insert(m);
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