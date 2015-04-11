#ifndef VR_OPENGL_H_
#define VR_OPENGL_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#endif

#include "glext.h"

#ifdef __unix__
#include <GL/glx.h>
#endif

void vrimp_swap_buffers(void);

#if defined(__unix__)
#define vrimp_glfunc(n) glXGetProcAddress(n)
#elif defined(WIN32)
#define vrimp_glfunc(n)	wglGetProcAddress(n)
#endif

#endif	/* VR_OPENGL_H_ */
