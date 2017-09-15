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
#ifdef USE_MOD_OPENHMD

#include "mod_openhmd.h"
#include "goatvr_impl.h"
#include "opengl.h"

REG_MODULE(openhmd, ModuleOpenHMD)

using namespace goatvr;

ModuleOpenHMD::ModuleOpenHMD()
{
	ohmd = 0;
	rtex_valid = false;
}

ModuleOpenHMD::~ModuleOpenHMD()
{
	destroy();
}

bool ModuleOpenHMD::init()
{
	if(!Module::init()) {
		return false;
	}

	if(!(ohmd = ohmd_ctx_create())) {
		print_error("failed to create OpenHMD context\n");
		return false;
	}
	return true;
}

void ModuleOpenHMD::destroy()
{
	ohmd_ctx_destroy(ohmd);
	ohmd = 0;

	Module::destroy();
}

enum goatvr_module_type ModuleOpenHMD::get_type() const
{
	return GOATVR_DISPLAY_MODULE;
}

const char *ModuleOpenHMD::get_name() const
{
	return "openhmd";
}

bool ModuleOpenHMD::detect()
{
	print_info("detecting HMD devices\n");

	int ndev = ohmd_ctx_probe(ohmd);
	if(ndev <= 0) {
		print_info("no HMDs detected\n");
		avail = false;
		return false;
	}

	for(int i=0; i<ndev; i++) {
		const char *vendor = ohmd_list_gets(ohmd, i, OHMD_VENDOR);
		const char *product = ohmd_list_gets(ohmd, i, OHMD_PRODUCT);
		const char *path = ohmd_list_gets(ohmd, i, OHMD_PATH);
		print_info("HMD found: %s - %s [%s]\n", vendor, product, path);
	}

	avail = true;
	return true;
}

bool ModuleOpenHMD::start()
{
	if(dev) return true;	// already started

	if(!(dev = ohmd_list_open_device(ohmd, 0))) {
		print_error("failed to open HMD device 0\n");
		return false;
	}

	// force creation of the render target
	get_render_texture();
	return true;
}

void ModuleOpenHMD::stop()
{
	if(!dev) return;

	if(rtex.tex) {
		glDeleteTextures(1, &rtex.tex);
		rtex.tex = 0;
	}
	rtex_valid = false;

	ohmd_close_device(dev);
	dev = 0;
}

void ModuleOpenHMD::update()
{
	if(!dev) return;
	float units_scale = goatvr_get_units_scale();

	ohmd_ctx_update(ohmd);

	for(int i=0; i<2; i++) {
		ohmd_device_getf(dev, (ohmd_float_value)(OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX + i), eye[i].xform[0]);
	}

	ohmd_device_getf(dev, OHMD_POSITION_VECTOR, &head.pos.x);
	head.pos.x *= units_scale;
	head.pos.y *= units_scale;
	head.pos.z *= units_scale;
	ohmd_device_getf(dev, OHMD_ROTATION_QUAT, &head.rot.x);
}

void ModuleOpenHMD::recenter()
{
	const static float zero[] = {0, 0, 0, 1};
	if(dev) {
		ohmd_device_setf(dev, OHMD_POSITION_VECTOR, zero);
		ohmd_device_setf(dev, OHMD_ROTATION_QUAT, zero);
	}
}

bool ModuleOpenHMD::have_headtracking() const
{
	return true;
}

void ModuleOpenHMD::set_fbsize(int width, int height, float fbscale)
{
	if(fbscale != rtex.fbscale) {
		rtex_valid = false;
	}
	rtex.fbscale = fbscale;
}

RenderTexture *ModuleOpenHMD::get_render_texture()
{
	if(!dev) return &rtex;

	if(!rtex_valid) {
		int fbwidth, fbheight;

		ohmd_device_geti(dev, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &fbwidth);
		ohmd_device_geti(dev, OHMD_SCREEN_VERTICAL_RESOLUTION, &fbheight);

		rtex.update(fbwidth, fbheight);
		// TODO more
		rtex_valid = true;
	}
	return &rtex;
}

void ModuleOpenHMD::draw_start()
{
}

void ModuleOpenHMD::draw_done()
{
}

void ModuleOpenHMD::draw_mirror()
{
}

void ModuleOpenHMD::get_view_matrix(Mat4 &mat, int eye) const
{
	mat = eye_inv_xform[eye];
}

void ModuleOpenHMD::get_proj_matrix(Mat4 &mat, int eye, float znear, float zfar) const
{
	ohmd_device_setf(dev, OHMD_PROJECTION_ZNEAR, &znear);
	ohmd_device_setf(dev, OHMD_PROJECTION_ZFAR, &zfar);
	ohmd_device_getf(dev, (ohmd_float_value)(OHMD_LEFT_EYE_GL_PROJECTION_MATRIX + eye), mat[0]);
}

Vec3 ModuleOpenHMD::get_head_position() const
{
	return head.pos;
}

Quat ModuleOpenHMD::get_head_orientation() const
{
	return head.rot;
}

void ModuleOpenHMD::get_head_matrix(Mat4 &mat) const
{
	// TODO
}

#else

#include "module.h"
// this expands to an empty register_mod_openhmd() function
NOREG_MODULE(openhmd)

#endif	// USE_MOD_OPENHMD
