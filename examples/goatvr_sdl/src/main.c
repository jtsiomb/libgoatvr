#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "goatvr.h"

static int init(void);
static void cleanup(void);
static void draw(void);
static void reshape(int x, int y);
static void handle_event(SDL_Event *ev);
static void handle_key(int key, int press);

static SDL_Window *win;
static SDL_GLContext ctx;
static int width, height;
static int done;

int main(int argc, char **argv)
{
	int pos = SDL_WINDOWPOS_UNDEFINED;
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "failed to initialize SDL\n");
		goto quit;
	}
	if(!(win = SDL_CreateWindow("libgoatvr sdl test", pos, pos, 800, 600, SDL_WINDOW_OPENGL))) {
		fprintf(stderr, "failed to create window\n");
		goto quit;
	}

	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	if(!(ctx = SDL_GL_CreateContext(win))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		goto quit;
	}

	if(init() == -1) {
		goto quit;
	}

	SDL_GetWindowSize(win, &width, &height);
	reshape(width, height);

	for(;;) {
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			handle_event(&ev);
			if(done) {
				goto quit;
			}
		}
		draw();
	}

quit:
	cleanup();
	return 0;
}

static int init(void)
{
	glClearColor(1, 0, 0, 1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if(goatvr_init() == -1) {
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	goatvr_shutdown();

	if(ctx) {
		SDL_GL_DeleteContext(ctx);
	}
	if(win) {
		SDL_DestroyWindow(win);
	}
	SDL_Quit();
}

static void draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SDL_GL_SwapWindow(win);
	assert(glGetError() == GL_NO_ERROR);
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
}

static void handle_event(SDL_Event *ev)
{
	switch(ev->type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		handle_key(ev->key.keysym.sym, ev->key.state == SDL_PRESSED ? 1 : 0);
		break;

	case SDL_WINDOWEVENT:
		if(ev->window.event == SDL_WINDOWEVENT_RESIZED) {
			int new_width = ev->window.data1;
			int new_height = ev->window.data2;

			if(new_width != width || new_height != height) {
				width = new_width;
				height = new_height;
				reshape(width, height);
			}
		}
		break;
	}
}

static void handle_key(int key, int press)
{
	if(press) {
		switch(key) {
		case SDLK_ESCAPE:
			done = 1;
			break;
		}
	}
}
