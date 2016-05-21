#ifndef OPENGL_H_
#define OPENGL_H_

#ifdef WIN32

#ifndef WINGDIAPI
#define WINGDIAPI __declspec(dllimport)
#endif
#ifndef APIENTRY
#define APIENTRY __stdcall
#endif

#endif	/* WIN32 */

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#endif

namespace goatvr {

bool init_opengl();

#ifndef GL_VERSION_3_0
/* ARB_framebuffer_object / EXT_framebuffer_object */
#define GL_DEPTH_COMPONENT24	0x81a6
#define GL_FRAMEBUFFER			0x8d40
#define GL_RENDERBUFFER			0x8d41
#define GL_COLOR_ATTACHMENT0	0x8ce0
#define GL_DEPTH_ATTACHMENT		0x8d00
#define GL_STENCIL_ATTACHMENT	0x8d20
#define GL_FRAMEBUFFER_COMPLETE	0x8cd5

typedef void (*GLGenFramebuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (*GLDeleteFramebuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (*GLBindFramebufferFunc)(GLenum target, GLuint fbo);
typedef void (*GLFramebufferTexture2DFunc)(GLenum target, GLenum attachment, GLenum textarget, GLuint tex, GLint level);
typedef void (*GLFramebufferRenderbufferFunc)(GLenum target, GLenum attachment, GLenum rbtarget, GLuint rbuf);
typedef void (*GLGenRenderbuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (*GLDeleteRenderbuffersFunc)(GLsizei n, GLuint *bufs);
typedef void (*GLBindRenderbufferFunc)(GLenum target, GLuint rbuf);
typedef void (*GLRenderbufferStorageFunc)(GLenum target, GLenum ifmt, GLsizei width, GLsizei height);
typedef GLenum (*GLCheckFramebufferStatusFunc)(GLenum target);

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

#endif	/* OPENGL_H_ */
