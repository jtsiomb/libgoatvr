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
#endif	/* __unix__ */

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
#endif	/* WIN32 */

#ifdef __APPLE__
void vrimp_swap_buffers(void)
{
	/* TODO: I don't think this can even be done without obj-c and a pointer
	 * to a GLView class or whatever the fuck it's called... investigate further
	 */
}


void (*vrimp_glfunc(const char *name))()
{
	/* TODO: whatever */
	return 0;
}
#endif	/* __APPLE__ */
