#ifndef GOATVR_IMPL_H_
#define GOATVR_IMPL_H_

#include <vector>
#include "module.h"
#include "device.h"

typedef goatvr::Module goatvr_module;
typedef goatvr::Device goatvr_device;

#define LIBGOATVR_IMPL
#include "goatvr.h"

namespace goatvr {

struct GoatVR {
	std::vector<Module*> modules;
	std::vector<Device*> devices;

	bool vr_mode;
};

extern GoatVR *vr;	// global goatvr state object

}

#endif	/* GOATVR_IMPL_H_ */
