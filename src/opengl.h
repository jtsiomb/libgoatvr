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
#ifndef OPENGL_H_
#define OPENGL_H_

#include <assert.h>

#ifdef WIN32

#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif

#define GLAPI APIENTRY
#else
#define GLAPI
#endif	/* WIN32 */

#ifdef __APPLE__
#include <OpenGL/gl.h>

/* define all GL versions we need, to let us use the functions directly */
#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1
#endif

#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#endif

namespace goatvr {

bool init_opengl();

#ifndef GL_VERSION_2_0
typedef void (GLAPI *GLUseProgramFunc)(GLuint prog);

extern GLUseProgramFunc glUseProgram;
#endif	// !GL_VERSION_2_0

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24	0x81a6
#endif

#ifndef GL_SRGB
#define GL_SRGB 0x8c40
#endif

#ifndef GL_VERSION_3_0
/* ARB_framebuffer_object / EXT_framebuffer_object */
#define GL_FRAMEBUFFER			0x8d40
#define GL_RENDERBUFFER			0x8d41
#define GL_COLOR_ATTACHMENT0	0x8ce0
#define GL_DEPTH_ATTACHMENT		0x8d00
#define GL_STENCIL_ATTACHMENT	0x8d20
#define GL_FRAMEBUFFER_COMPLETE	0x8cd5

typedef void (GLAPI *GLGenFramebuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (GLAPI *GLDeleteFramebuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (GLAPI *GLBindFramebufferFunc)(GLenum target, GLuint fbo);
typedef void (GLAPI *GLFramebufferTexture2DFunc)(GLenum target, GLenum attachment, GLenum textarget, GLuint tex, GLint level);
typedef void (GLAPI *GLFramebufferRenderbufferFunc)(GLenum target, GLenum attachment, GLenum rbtarget, GLuint rbuf);
typedef void (GLAPI *GLGenRenderbuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (GLAPI *GLDeleteRenderbuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (GLAPI *GLBindRenderbufferFunc)(GLenum target, GLuint rbuf);
typedef void (GLAPI *GLRenderbufferStorageFunc)(GLenum target, GLenum ifmt, GLsizei width, GLsizei height);
typedef GLenum (GLAPI *GLCheckFramebufferStatusFunc)(GLenum target);

extern GLGenFramebuffersFunc glGenFramebuffers;
extern GLDeleteFramebuffersFunc glDeleteFramebuffers;
extern GLBindFramebufferFunc glBindFramebuffer;
extern GLFramebufferTexture2DFunc glFramebufferTexture2D;
extern GLFramebufferRenderbufferFunc glFramebufferRenderbuffer;
extern GLGenRenderbuffersFunc glGenRenderbuffers;
extern GLDeleteRenderbuffersFunc glDeleteRenderbuffers;
extern GLBindRenderbufferFunc glBindRenderbuffer;
extern GLRenderbufferStorageFunc glRenderbufferStorage;
extern GLCheckFramebufferStatusFunc glCheckFramebufferStatus;
#endif	// !GL_VERSION_3_0

}	// namespace goatvr

#define CHECK_GLERROR	assert(glGetError() == GL_NO_ERROR)

#endif	/* OPENGL_H_ */
