#include <stdio.h>
#include "module.h"
#include "modman.h"

using namespace goatvr;

Module::Module()
{
	avail = act = false;

	add_module(this);	// register ourselves
}

Module::~Module()
{
	destroy();
}

bool Module::init()
{
	return true;
}

void Module::destroy()
{
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

void Module::update()
{
}

void Module::set_fbsize(int width, int height)
{
}

RenderTexture *Module::get_render_texture(float s)
{
	return 0;
}

void Module::draw_eye(int eye)
{
}

void Module::draw_done()
{
}

Mat4 Module::get_view_matrix(int eye)
{
	return Mat4::identity;
}

Mat4 Module::get_proj_matrix(int eye)
{
	return Mat4::identity;
}
