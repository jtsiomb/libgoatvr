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
			if(m->get_type() == MODULE_RENDERING) {
				int p = m->get_priority();
				printf(" (render module, priority: %d)\n", p);

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
	if(!render_module) {
		if(!rmod) {
			printf("no usable render module found!\n");
		} else {
			// activate the highest priority render module
			activate(rmod);
			printf("activating rendering module: %s\n", rmod->get_name());
		}
	}
}

static goatvr::Source *find_tracking_source(const char *name)
{
	goatvr::Source *s;
	if(!(s = goatvr_find_source(name))) {
		fprintf(stderr, "Input source %s is not available\n", name);
		return 0;
	}
	if(!goatvr_source_spatial(s)) {
		fprintf(stderr, "Input source %s cannot be used for tracking\n", name);
		return 0;
	}
	return s;
}

void goatvr::configure_tracking()
{
	if(!render_module) {
		// shouldn't be called without a render module at all really
		fprintf(stderr, "goatvr bug: configure_tracking called without an active render module\n");
		return;
	}

	Source *head_src = 0;
	Source *hand_src = 0;
	int nsrc = goatvr_num_sources();

	char *env_head = getenv("GOATVR_HEAD_TRACKER");
	if(env_head) {
		printf("GOATVR_HEAD_TRACKER is set, looking for input source %s to use\n", env_head);
		head_src = find_tracking_source(env_head);
	}

	char *env_hand = getenv("GOATVR_HAND_TRACKER");
	if(env_hand) {
		printf("GOATVR_HAND_TRACKER is set, looking for input source %s to use\n", env_hand);
		hand_src = find_tracking_source(env_hand);
	}


	if(!head_src && !render_module->have_head_tracking()) {
		/* if the render module doesn't have head-tracking, try to find a suitable
		 * tracking source, even if not specified as an env-var
		 */
		for(int i=0; i<nsrc; i++) {
			Source *s = goatvr_get_source(i);
			if(goatvr_source_spatial(s) && s != hand_src) {
				head_src = s;
				break;
			}
		}

		if(head_src) {
			printf("Render module %s doesn't have head tracking. Using input source %s\n",
					render_module->get_name(), goatvr_source_name(head_src));
		}
	}
	goatvr_set_head_tracker(head_src);

	if(!hand_src && !render_module->have_hand_tracking()) {
		/* if the render module doesn't have hand-tracking, try to find a suitable
		 * tracking source, even if not specified as an env-var
		 */
		for(int i=0; i<nsrc; i++) {
			Source *s = goatvr_get_source(i);
			if(goatvr_source_spatial(s) && s != head_src) {
				hand_src = s;
				break;
			}
		}

		if(hand_src) {
			printf("Render module %s doesn't have hand tracking. Using input source %s\n",
					render_module->get_name(), goatvr_source_name(hand_src));
		}
	}
	goatvr_set_hand_tracker(GOATVR_SRC_RIGHT_HAND, hand_src);
}
