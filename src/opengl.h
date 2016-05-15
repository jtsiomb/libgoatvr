#ifndef OPENGL_H_
#define OPENGL_H_

#ifdef WIN32

#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif

#endif	// WIN32

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#endif	// OPENGL_H_
