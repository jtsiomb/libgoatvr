#ifndef VR_OPENGL_H_
#define VR_OPENGL_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef __unix__
#include <GL/glx.h>
#endif

void vrimp_swap_buffers(void);

void (*vrimp_glfunc(const char *name))();

#endif	/* VR_OPENGL_H_ */
