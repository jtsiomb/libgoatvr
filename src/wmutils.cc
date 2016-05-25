#include <stdio.h>
#include <stdint.h>
#include "wmutils.h"

#if defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

static void init();
static void sdl_hmd_fullscreen(int xpos, int ypos);
static void glut_hmd_fullscreen(int xpos, int ypos);

/* SDL2 stuff */
#define SDL_WINDOW_FULLSCREEN_DESKTOP	0x1001

static void *(*SDL_GL_GetCurrentWindow)();
static int (*SDL_SetWindowFullscreen)(void *win, uint32_t flags);
static void (*SDL_SetWindowPosition)(void *win, int x, int y);

/* GLUT stuff */
static void (*glutPositionWindow)(int x, int y);
static void (*glutFullScreen)();


namespace goatvr {

void hmd_fullscreen(int xpos, int ypos)
{
	static bool did_init;
	if(!did_init) {
		init();
		did_init = true;
	}

	if(SDL_SetWindowFullscreen) {
		sdl_hmd_fullscreen(xpos, ypos);
	} else if(glutFullScreen) {
		glut_hmd_fullscreen(xpos, ypos);
	}
}

}	// namespace goatvr

static void init()
{
#if defined(__unix__) || defined(__APPLE__)
	SDL_GL_GetCurrentWindow = (void *(*)())dlsym(RTLD_DEFAULT, "SDL_GL_GetCurrentWindow");
	SDL_SetWindowFullscreen = (int (*)(void*, uint32_t))dlsym(RTLD_DEFAULT, "SDL_SetWindowFullscreen");
	SDL_SetWindowPosition = (void (*)(void*, int, int))dlsym(RTLD_DEFAULT, "SDL_SetWindowPosition");

	glutFullScreen = (void (*)())dlsym(RTLD_DEFAULT, "glutFullScreen");
	glutPositionWindow = (void (*)(int, int))dlsym(RTLD_DEFAULT, "glutPositionWindow");
#endif
	printf("goatvr: hmd_fullscreen hack init: ");
	if(SDL_SetWindowFullscreen) {
		printf("SDL\n");
	} else if(glutFullScreen) {
		printf("glut\n");
	} else {
		printf("failed\n");
	}
}

static void sdl_hmd_fullscreen(int xpos, int ypos)
{
	void *win = SDL_GL_GetCurrentWindow();
	if(!win) {
		fprintf(stderr, "goatvr: fullscreen hack (SDL): failed to get current window\n");
	}
	SDL_SetWindowPosition(win, xpos, ypos);
	SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

static void glut_hmd_fullscreen(int xpos, int ypos)
{
	glutPositionWindow(xpos, ypos);
	glutFullScreen();
}
