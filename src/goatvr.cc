#include "goatvr_impl.h"

using namespace goatvr;

extern "C" {

int goatvr_init()
{
	return -1;
}

void goatvr_shutdown()
{
}

void goatvr_detect()
{
}

void goatvr_startvr()
{
}

void goatvr_stopvr()
{
}

int goatvr_invr()
{
	return 0;
}

}	// extern "C"


unsigned int goatvr::next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
