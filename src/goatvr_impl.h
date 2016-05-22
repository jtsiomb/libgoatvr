#ifndef GOATVR_IMPL_H_
#define GOATVR_IMPL_H_

#include <vector>

namespace goatvr {

class Module;
class Device;

}

typedef goatvr::Module goatvr_module;
typedef goatvr::Device goatvr_device;

#define LIBGOATVR_IMPL
#include "goatvr.h"

namespace goatvr {

unsigned int next_pow2(unsigned int x);

}

#endif	/* GOATVR_IMPL_H_ */
