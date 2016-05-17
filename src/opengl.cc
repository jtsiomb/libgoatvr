#include <stdio.h>
#include "opengl.h"

typedef int (*glfunc_type)();
static glfunc_type load_glext(const char *name);

namespace goatvr {

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
	return (glfunc_type)glXGetProcAddress(name);
}

#else

glfunc_type load_glext(const char *name)
{
	return 0;
}
#endif