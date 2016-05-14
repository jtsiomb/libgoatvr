#include <string.h>
#include "opengl.h"
#include "mod_oculus.h"

using namespace goatvr;

ModuleOculus::ModuleOculus()
{
	memset(&rtex, 0, sizeof rtex);
}

bool ModuleOculus::init()
{
	if(!Module::init()) {
		return false;
	}

	if(ovr_Initialize(0) != 0) {
		print_error("failed to initialize libovr\n");
		return false;
	}

	glGenTextures(1, &rtex.tex);
	return true;
}

void ModuleOculus::destroy()
{
	ovr_Shutdown();

	if(rtex.tex) {
		glDeleteTextures(1, &rtex.tex);
		rtex.tex = 0;
	}
	Module::destroy();
}

ModuleType ModuleOculus::get_type() const
{
	return MODULE_RENDERING;
}

const char *ModuleOculus::get_name() const
{
	return "oculus";
}

void ModuleOculus::detect()
{
	print_info("running HMD detection\n");

	ovrHmdDesc hmd = ovr_GetHmdDesc(0);
	if(hmd.Type == ovrHmd_None) {
		print_info("none available\n");
		avail = false;
		return;
	}

	print_info("HMD found: %s - %s\n", hmd.Manufacturer, hmd.ProductName);
	avail = true;
}

void ModuleOculus::start()
{
	if(ovr) return;		// already started

	if(ovr_Create(&ovr, &ovr_luid) != 0) {
		print_error("failed to create oculus session\n");
		return;
	}
}

void ModuleOculus::stop()
{
	if(!ovr) return;	// not started

	ovr_Destroy(ovr);
	ovr = 0;
}
