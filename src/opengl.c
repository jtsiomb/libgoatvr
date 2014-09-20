#include "opengl.h"

#ifdef __unix__
void vrimp_swap_buffers(void)
{
	Display *dpy = glXGetCurrentDisplay();
	Drawable win = glXGetCurrentDrawable();
	glXSwapBuffers(dpy, win);
}

void (*vrimp_glfunc(const char *name))()
{
	return glXGetProcAddress((const unsigned char*)name);
}
#endif

#ifdef WIN32
void vrimp_swap_buffers(void)
{
	HDC dc = wglGetCurrentDC();
	SwapBuffers(dc);
}

void (*vrimp_glfunc(const char *name))()
{
	return wglGetProcAddress(name);
}
#endif
