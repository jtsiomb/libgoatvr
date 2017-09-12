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
#include <stdio.h>
#include <stdlib.h>
#include "autocfg.h"
#include "goatvr_impl.h"
#include "modman.h"

void goatvr::activate_module()
{
	char *rmod_env = getenv("GOATVR_MODULE");
	if(rmod_env) {
		printf("GOATVR_MODULE is set, looking for render module %s to activate.\n", rmod_env);
	}

	Module *rmod = 0;
	int max_prio = -1;

	printf("goatvr: detected %d usable modules:\n", get_num_usable_modules());
	for(int i=0; i<get_num_modules(); i++) {
		Module *m = get_module(i);
		if(m->usable()) {
			printf("  %s", m->get_name());
			if(m->get_type() == GOATVR_DISPLAY_MODULE) {
				int p = m->get_priority();
				printf(" (display module, priority: %d)\n", p);

				if(rmod_env) {
					// user asked for a specific module
					if(strcasecmp(m->get_name(), rmod_env) == 0) {
						rmod = m;
					}
				} else {
					// otherwise go by priority
					if(p > max_prio) {
						rmod = m;
						max_prio = p;
					}
				}

			} else {
				putchar('\n');
				activate(m);	// activate all usable non-rendering modules
			}
		}
	}

	// only do the following if we haven't already activated a render module
	if(!display_module) {
		if(!rmod) {
			printf("no usable render module found!\n");
		} else {
			// activate the highest priority render module
			activate(rmod);
			printf("activating rendering module: %s\n", rmod->get_name());
		}
	}
}
