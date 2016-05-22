#include <stdio.h>
#include <stdarg.h>
#include "module.h"
#include "modman.h"

using namespace goatvr;

Module::Module()
{
	avail = act = false;
	prio = 0;
}

Module::~Module()
{
}

bool Module::init()
{
	return true;
}

void Module::destroy()
{
}

void Module::set_priority(int p)
{
	prio = p;
}

int Module::get_priority() const
{
	return prio;
}

bool Module::detect()
{
	return false;
}

bool Module::usable() const
{
	return avail;
}

void Module::activate()
{
	act = true;
}

void Module::deactivate()
{
	act = false;
}

bool Module::active() const
{
	return act;
}

void Module::start()
{
}

void Module::stop()
{
}

void Module::set_origin_mode(goatvr_origin_mode mode)
{
}

void Module::update()
{
}

void Module::set_fbsize(int width, int height, float fbscale)
{
}

RenderTexture *Module::get_render_texture()
{
	return 0;
}

void Module::draw_start()
{
}

void Module::draw_eye(int eye)
{
}

void Module::draw_done()
{
}

void Module::draw_mirror()
{
}

Mat4 Module::get_view_matrix(int eye)
{
	return Mat4::identity;
}

Mat4 Module::get_proj_matrix(int eye, float znear, float zfar)
{
	return Mat4::identity;
}

void Module::print_info(const char *fmt, ...) const
{
	va_list ap;

	printf("module %s: ", get_name());

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void Module::print_error(const char *fmt, ...) const
{
	va_list ap;

	fprintf(stderr, "module %s: ", get_name());

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
