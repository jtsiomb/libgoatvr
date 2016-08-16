/*
GoatVR - a modular virtual reality abstraction library
Copyright (C) 2014-2016  John Tsiombikas <nuclear@member.fsf.org>

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
#include "opengl.h"

#ifndef __APPLE__
typedef int (*glfunc_type)();
static glfunc_type load_glext(const char *name);
#endif

namespace goatvr {

#ifndef GL_VERSION_2_0
GLUseProgramFunc glUseProgram;
#endif

#ifndef GL_VERSION_3_0
GLGenFramebuffersFunc glGenFramebuffers;
GLDeleteFramebuffersFunc glDeleteFramebuffers;
GLBindFramebufferFunc glBindFramebuffer;
GLFramebufferTexture2DFunc glFramebufferTexture2D;
GLFramebufferRenderbufferFunc glFramebufferRenderbuffer;
GLGenRenderbuffersFunc glGenRenderbuffers;
GLDeleteRenderbuffersFunc glDeleteRenderbuffers;
GLBindRenderbufferFunc glBindRenderbuffer;
GLRenderbufferStorageFunc glRenderbufferStorage;
GLCheckFramebufferStatusFunc glCheckFramebufferStatus;
#endif

bool init_opengl()
{
#ifndef GL_VERSION_2_0
	glUseProgram = (GLUseProgramFunc)load_glext("glUseProgram");
#endif	// !GL_VERSION_2_0

#ifndef GL_VERSION_3_0
	glGenFramebuffers = (GLGenFramebuffersFunc)load_glext("glGenFramebuffersEXT");
	glDeleteFramebuffers = (GLDeleteFramebuffersFunc)load_glext("glDeleteFramebuffersEXT");
	glBindFramebuffer = (GLBindFramebufferFunc)load_glext("glBindFramebufferEXT");
	glFramebufferTexture2D = (GLFramebufferTexture2DFunc)load_glext("glFramebufferTexture2DEXT");
	glFramebufferRenderbuffer = (GLFramebufferRenderbufferFunc)load_glext("glFramebufferRenderbufferEXT");
	glGenRenderbuffers = (GLGenRenderbuffersFunc)load_glext("glGenRenderbuffersEXT");
	glDeleteRenderbuffers = (GLDeleteRenderbuffersFunc)load_glext("glDeleteRenderbuffersEXT");
	glBindRenderbuffer = (GLBindRenderbufferFunc)load_glext("glBindRenderbufferEXT");
	glRenderbufferStorage = (GLRenderbufferStorageFunc)load_glext("glRenderbufferStorageEXT");
	glCheckFramebufferStatus = (GLCheckFramebufferStatusFunc)load_glext("glCheckFramebufferStatusEXT");
	if(!glGenFramebuffers) {
		fprintf(stderr, "failed to load the framebuffer object extension\n");
		return false;
	}
#endif	// !GL_VERSION_3_0
	return true;
}

}	// namespace goatvr

#ifdef WIN32
#undef APIENTRY
#undef WINGDIAPI
#include <windows.h>

glfunc_type load_glext(const char *name)
{
	return (glfunc_type)wglGetProcAddress(name);
}

#elif defined(__unix__)
#include <GL/glx.h>

glfunc_type load_glext(const char *name)
{
	return (glfunc_type)glXGetProcAddress((unsigned char*)name);
}
#endif
