#ifndef MODMAN_H_
#define MODMAN_H_

#include <vector>
#include "module.h"

namespace goatvr {

extern Module *render_module;

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
void start();
void stop();
void update();

// operations to be performed on the active rendering module
void draw_start();
void draw_eye(int eye);
void draw_done();

}	// namespace goatvr

#endif	// MODMAN_H_
