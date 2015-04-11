#include "opengl.h"

#ifdef __unix__
void vrimp_swap_buffers(void)
{
	Display *dpy = glXGetCurrentDisplay();
	Drawable win = glXGetCurrentDrawable();
	glXSwapBuffers(dpy, win);
}
#endif	/* __unix__ */

#ifdef WIN32
void vrimp_swap_buffers(void)
{
	HDC dc = wglGetCurrentDC();
	SwapBuffers(dc);
}
#endif	/* WIN32 */

#ifdef __APPLE__
void vrimp_swap_buffers(void)
{
	/* TODO: I don't think this can even be done without obj-c and a pointer
	 * to a GLView class or whatever the fuck it's called... investigate further
	 */
}
#endif	/* __APPLE__ */
