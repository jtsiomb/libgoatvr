#include "opengl.h"

#ifdef __unix__
void vr_gl_swap_buffers(void)
{
	Display *dpy = glXGetCurrentDisplay();
	Drawable win = glXGetCurrentDrawable();
	glXSwapBuffers(dpy, win);
}
#endif

#ifdef WIN32
void vr_gl_swap_buffers(void)
{
	HDC dc = wglGetCurrentDC();
	SwapBuffers(dc);
}
#endif
