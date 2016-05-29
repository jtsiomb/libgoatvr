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
#include <stdio.h>
#include <stdint.h>
#include "wmutils.h"

#if defined(__unix__) || defined(__APPLE__)
#include <dlfcn.h>
#endif

static bool did_init;
#define INIT_ONCE	\
	do { \
		if(!did_init) { \
			init(); \
			did_init = true; \
		} \
	} while(0)

static void init();
static void sdl_hmd_fullscreen(int xpos, int ypos);
static void glut_hmd_fullscreen(int xpos, int ypos);

/* SDL2 stuff */
#define SDL_WINDOW_FULLSCREEN_DESKTOP	0x1001

static void *(*SDL_GL_GetCurrentWindow)();
static int (*SDL_SetWindowFullscreen)(void *win, uint32_t flags);
static void (*SDL_SetWindowPosition)(void *win, int x, int y);
static void (*SDL_SetWindowSize)(void *win, int w, int h);

/* GLUT stuff */
static void (*glutPositionWindow)(int x, int y);
static void (*glutReshapeWindow)(int x, int y);
static void (*glutFullScreen)();


namespace goatvr {

void hmd_fullscreen(int xpos, int ypos)
{
	INIT_ONCE;

	if(SDL_SetWindowFullscreen) {
		sdl_hmd_fullscreen(xpos, ypos);
	} else if(glutFullScreen) {
		glut_hmd_fullscreen(xpos, ypos);
	}
}

void win_resize(int width, int height)
{
	INIT_ONCE;

	if(SDL_SetWindowSize) {
		void *win = SDL_GL_GetCurrentWindow();
		if(win) {
			SDL_SetWindowSize(win, width, height);
		}
	} else if(glutReshapeWindow) {
		glutReshapeWindow(width, height);
	}
}

}	// namespace goatvr

static void init()
{
#if defined(__unix__) || defined(__APPLE__)
	SDL_GL_GetCurrentWindow = (void *(*)())dlsym(RTLD_DEFAULT, "SDL_GL_GetCurrentWindow");
	SDL_SetWindowFullscreen = (int (*)(void*, uint32_t))dlsym(RTLD_DEFAULT, "SDL_SetWindowFullscreen");
	SDL_SetWindowPosition = (void (*)(void*, int, int))dlsym(RTLD_DEFAULT, "SDL_SetWindowPosition");
	SDL_SetWindowSize = (void (*)(void*, int, int))dlsym(RTLD_DEFAULT, "SDL_SetWindowSize");

	glutFullScreen = (void (*)())dlsym(RTLD_DEFAULT, "glutFullScreen");
	glutPositionWindow = (void (*)(int, int))dlsym(RTLD_DEFAULT, "glutPositionWindow");
	glutReshapeWindow = (void (*)(int, int))dlsym(RTLD_DEFAULT, "glutReshapeWindow");
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
		return;
	}
	SDL_SetWindowPosition(win, xpos, ypos);
	SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

static void glut_hmd_fullscreen(int xpos, int ypos)
{
	glutPositionWindow(xpos, ypos);
	glutFullScreen();
}
