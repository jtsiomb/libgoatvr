#ifndef MODMAN_H_
#define MODMAN_H_

#include <vector>
#include "module.h"

namespace goatvr {

void add_module(Module *m);
int get_num_modules();
Module *get_module(int idx);
Module *find_module(const char *name);

void activate(Module *m);
void deactivate(Module *m);

// vr operations to be performed on all active modules
void start();
void stop();
void detect();
void update();
void draw_eye(int eye);
void draw_done();

}	// namespace goatvr

#endif	// MODMAN_H_