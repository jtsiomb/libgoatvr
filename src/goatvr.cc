#include "goatvr_impl.h"

using namespace goatvr;

GoatVR *goatvr::vr;

extern "C" {

int goatvr_init()
{
	if(vr) return 0;	// already initialized

	try {
		vr = new GoatVR;
	}
	catch(...) {
		return -1;
	}
	return 0;
}

void goatvr_shutdown()
{
	if(vr) {
		delete vr;
		vr = 0;
	}
}

void goatvr_detect()
{
	for(size_t i=0; i<vr->modules.size(); i++) {
		vr->modules[i]->detect();
	}
}

void goatvr_startvr()
{
	if(vr->vr_mode) return;

	for(size_t i=0; i<vr->modules.size(); i++) {
		Module *m = vr->modules[i];
		if(m->active()) {
			m->start();
		}
	}
	vr->vr_mode = true;
}

void goatvr_stopvr()
{
	if(!vr->vr_mode) return;

	for(size_t i=0; i<vr->modules.size(); i++) {
		vr->modules[i]->stop();
	}
	vr->vr_mode = false;
}

int goatvr_invr()
{
	return vr->vr_mode ? 1 : 0;
}

}	// extern "C"
