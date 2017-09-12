#ifndef INPMAN_H_
#define INPMAN_H_

#include <string>
#include "goatvr_impl.h"

namespace goatvr {

struct InputButton {
	int idx;
	std::string name;
	Module *module;
	int mod_idx;
};

struct InputAxis {
	int idx;
	std::string name;
	Module *module;
	int mod_idx;
};

struct InputStick {
	int idx;
	std::string name;
	Module *module;
	int mod_idx;
};

// automatically adds/removes all inputs of a certain module
void inp_add_module(Module *mod);
void inp_remove_module(Module *mod);
void inp_clear();

int inp_num_buttons();
InputButton *inp_get_button(int idx);
InputButton *inp_find_button(const char *name);

int inp_num_axes();
InputAxis *inp_get_axis(int idx);
InputAxis *inp_find_axis(const char *name);

int inp_num_sticks();
InputStick *inp_get_stick(int idx);
InputStick *inp_find_stick(const char *name);

}	// namespace goatvr

#endif	// INPMAN_H_
