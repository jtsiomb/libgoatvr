#include <map>
#include <string>
#include "inpman.h"
#include "module.h"

namespace goatvr {

static std::vector<InputButton*> buttons;
static std::vector<InputAxis*> axes;
static std::vector<InputStick*> sticks;

static std::map<std::string, InputButton*> bn_map;
static std::map<std::string, InputAxis*> axis_map;
static std::map<std::string, InputStick*> stick_map;

void inp_add_module(Module *mod)
{
	int num_bn = mod->num_buttons();
	int base = (int)buttons.size();
	for(int i=0; i<num_bn; i++) {
		InputButton *bn = new InputButton;
		bn->idx = i + base;
		bn->module = mod;
		bn->mod_idx = i;
		bn->name = mod->get_button_name(i);
		buttons.push_back(bn);

		bn_map[bn->name] = bn;
	}

	int num_ax = mod->num_axes();
	base = (int)axes.size();
	for(int i=0; i<num_ax; i++) {
		InputAxis *axis = new InputAxis;
		axis->idx = i + base;
		axis->module = mod;
		axis->mod_idx = i;
		axis->name = mod->get_axis_name(i);
		axes.push_back(axis);

		axis_map[axis->name] = axis;
	}

	int num_st = mod->num_sticks();
	base = (int)sticks.size();
	for(int i=0; i<num_st; i++) {
		InputStick *stick = new InputStick;
		stick->idx = i + base;
		stick->module = mod;
		stick->mod_idx = i;
		stick->name = mod->get_stick_name(i);
		sticks.push_back(stick);

		stick_map[stick->name] = stick;
	}

	printf("goatvr: added input module with %d buttons, %d axes, and %d sticks\n",
			num_bn, num_ax, num_st);
}

void inp_remove_module(Module *mod)
{
	int num = (int)buttons.size();
	int start = -1;
	int end = num;
	for(int i=0; i<num; i++) {
		if(buttons[i]->module == mod) {
			if(start < 0) {
				start = i;
			}
			bn_map.erase(buttons[i]->name);
		} else if(start >= 0) {
			end = i;
			break;
		}
	}
	if(start >= 0) {
		for(int i=0; i<num; i++) {
			delete buttons[i + start];
		}
		std::vector<InputButton*>::iterator it_start = buttons.begin() + start;
		buttons.erase(it_start, it_start + end);
	}

	num = (int)axes.size();
	start = -1;
	end = num;
	for(int i=0; i<num; i++) {
		if(axes[i]->module == mod) {
			if(start < 0) {
				start = i;
			}
			axis_map.erase(axes[i]->name);
		} else if(start >= 0) {
			end = i;
			break;
		}
	}
	if(start >= 0) {
		for(int i=0; i<num; i++) {
			delete axes[i + start];
		}
		std::vector<InputAxis*>::iterator it_start = axes.begin() + start;
		axes.erase(it_start, it_start + end);
	}

	num = (int)sticks.size();
	start = -1;
	end = num;
	for(int i=0; i<num; i++) {
		if(sticks[i]->module == mod) {
			if(start < 0) {
				start = i;
			}
			stick_map.erase(sticks[i]->name);
		} else if(start >= 0) {
			end = i;
			break;
		}
	}
	if(start >= 0) {
		for(int i=0; i<num; i++) {
			delete sticks[i + start];
		}
		std::vector<InputStick*>::iterator it_start = sticks.begin() + start;
		sticks.erase(it_start, it_start + end);
	}
}

void inp_clear()
{
	buttons.clear();
	axes.clear();
	sticks.clear();
	bn_map.clear();
	axis_map.clear();
	stick_map.clear();
}

int inp_num_buttons()
{
	return (int)buttons.size();
}

InputButton *inp_get_button(int idx)
{
	if(idx < 0 || idx >= (int)buttons.size()) {
		return 0;
	}
	return buttons[idx];
}

InputButton *inp_find_button(const char *name)
{
	std::map<std::string, InputButton*>::const_iterator it = bn_map.find(name);
	return it == bn_map.end() ? 0 : it->second;
}

int inp_num_axes()
{
	return (int)axes.size();
}

InputAxis *inp_get_axis(int idx)
{
	if(idx < 0 || idx >= (int)axes.size()) {
		return 0;
	}
	return axes[idx];
}

InputAxis *inp_find_axis(const char *name)
{
	std::map<std::string, InputAxis*>::const_iterator it = axis_map.find(name);
	return it == axis_map.end() ? 0 : it->second;
}

int inp_num_sticks()
{
	return (int)sticks.size();
}

InputStick *inp_get_stick(int idx)
{
	if(idx < 0 || idx >= (int)sticks.size()) {
		return 0;
	}
	return sticks[idx];
}

InputStick *inp_find_stick(const char *name)
{
	std::map<std::string, InputStick*>::const_iterator it = stick_map.find(name);
	return it == stick_map.end() ? 0 : it->second;
}

}	// namespace goatvr
