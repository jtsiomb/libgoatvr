/* simple first person libgoatvr example program.
 * look for functions starting with the goatvr_ prefix, to see how to use
 * libgoatvr in the context of a simple VR program. All the calls to goatvr
 * are commented to explain their purpose and usage. Please also refer to
 * the comments in "goatvr.h" for more details.
 */
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "goatvr.h"

static int init(void);
static void cleanup(void);
static void draw(void);
static void draw_scene();
static void draw_box(float xsz, float ysz, float zsz, float norm_sign);
static void reshape(int x, int y);
static void handle_event(SDL_Event *ev);
static void handle_key(int key, int press);
static void handle_relative_mouse_motion(int dx, int dy);
static unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1);

static SDL_Window *win;
static SDL_GLContext ctx;
static int width, height;
static int done;
static int should_swap;

static unsigned int chess_tex;

static float cam_pos[3];
static float cam_theta, cam_phi;
static unsigned int start_time;
static int keystate[256];

static int use_mouselook = 1;

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

	SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
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
	int i;

	glEnable(GL_FRAMEBUFFER_SRGB);
	glGetError();	// ignore error if we don't have the extension
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	printf("OpenGL Renderer: %s\n", (char*)glGetString(GL_RENDERER));
	printf("OpenGL vendor: %s\n", (char*)glGetString(GL_VENDOR));
	printf("OpenGL Version: %s\n", (char*)glGetString(GL_VERSION));

	chess_tex = gen_chess_tex(1.0, 0.7, 0.4, 0.4, 0.7, 1.0);

	/* initialize goatvr and start VR mode */
	if(goatvr_init() == -1) {
		return -1;
	}
	/* GOATVR_HEAD means HMD-relative tracking, which is best for seated programs,
	 * the other option being GOATVR_FLOOR, for floor-referenced HMD position,
	 * which is best for standing programs.
	 */
	goatvr_set_origin_mode(GOATVR_HEAD);
	goatvr_startvr();
	/* NOTE: this is important. goatvr_should_swap tells us if the current VR module
	 * leaves backbuffer swapping to us (and we should call SDL_GL_SwapWindow) or
	 * handles the swapchain itself.
	 */
	should_swap = goatvr_should_swap();

	if(use_mouselook) {
		SDL_SetRelativeMouseMode(1);
		SDL_SetWindowGrab(win, 1);
	}

	start_time = SDL_GetTicks();
	return 0;
}

static void cleanup(void)
{
	goatvr_shutdown();

	glDeleteTextures(1, &chess_tex);

	if(ctx) {
		SDL_GL_DeleteContext(ctx);
	}
	if(win) {
		SDL_DestroyWindow(win);
	}
	SDL_Quit();
}

static void update(float dt)
{
	const float speed = 5.0;
	float dir_x = 0, dir_y = 0;

	if(keystate['w'] || keystate['W']) {
		dir_y += speed * dt;
	}
	if(keystate['s'] || keystate['S']) {
		dir_y -= speed * dt;
	}
	if(keystate['d'] || keystate['D']) {
		dir_x += speed * dt;
	}
	if(keystate['a'] || keystate['A']) {
		dir_x -= speed * dt;
	}

	float theta = cam_theta / 180.0 * M_PI;
	cam_pos[0] += dir_x * cos(theta) + dir_y * sin(theta);
	cam_pos[2] -= -dir_x * sin(theta) + dir_y * cos(theta);
}

static void draw(void)
{
	int i;
	unsigned int msec = SDL_GetTicks() - start_time;
	static unsigned int prev_msec;
	float dt = (msec - prev_msec) / 1000.0f;
	prev_msec = msec;

	update(dt);

	/* we must call goatvr_draw_start before we start drawing in VR */
	goatvr_draw_start();
	glClearColor(0.4, 0.14, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(i=0; i<2; i++) {
		/* before drawing the view from each eye, we need to call goatvr_draw_eye
		 * to let goatvr know which eye's view we are about to draw.
		 */
		goatvr_draw_eye(i);

		glMatrixMode(GL_PROJECTION);
		/* goatvr_projection_matrix returns a pointer to the projection matrix
		 * for a particular eye. The arguments are the eye index, the near clipping
		 * plane distance, and the far clipping plane distance.
		 */
		glLoadMatrixf(goatvr_projection_matrix(i, 0.05, 500.0));

		glMatrixMode(GL_MODELVIEW);
		/* goatvr_view_matrix gives us the view matrix (inverse transformation)
		 * for a particular eye. This matrix contains stereo separation, and
		 * head-tracking (if supported by the current VR module).
		 */
		glLoadMatrixf(goatvr_view_matrix(i));
		glRotatef(cam_phi, 1, 0, 0);
		glRotatef(cam_theta, 0, 1, 0);
		glTranslatef(-cam_pos[0], -cam_pos[1] - goatvr_get_eye_height(), -cam_pos[2]);

		draw_scene();
	}
	/* when we're done drawing both eyes, call goatvr_draw_done */
	goatvr_draw_done();

	/* Swap buffers if the current VR module doesn't handle the swappign itself.
	 * We called goatvr_should_swap() to determine that. See comment in init() above.
	 */
	if(should_swap) {
		SDL_GL_SwapWindow(win);
	}
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_scene()
{
	int i;
	float grey[] = {0.8, 0.8, 0.8, 1};
	float col[] = {0, 0, 0, 1};
	float lpos[][4] = {
		{-8, 2, 10, 1},
		{0, 15, 0, 1}
	};
	float lcol[][4] = {
		{0.8, 0.8, 0.8, 1},
		{0.4, 0.3, 0.3, 1}
	};

	for(i=0; i<2; i++) {
		glLightfv(GL_LIGHT0 + i, GL_POSITION, lpos[i]);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lcol[i]);
	}

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glTranslatef(0, 10, 0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, grey);
	glBindTexture(GL_TEXTURE_2D, chess_tex);
	glEnable(GL_TEXTURE_2D);
	draw_box(30, 20, 30, -1.0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	for(i=0; i<4; i++) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, grey);
		glPushMatrix();
		glTranslatef(i & 1 ? 5 : -5, 1, i & 2 ? -5 : 5);
		draw_box(0.5, 2, 0.5, 1.0);
		glPopMatrix();

		col[0] = i & 1 ? 1.0 : 0.3;
		col[1] = i == 0 ? 1.0 : 0.3;
		col[2] = i & 2 ? 1.0 : 0.3;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);

		glPushMatrix();
		if(i & 1) {
			glTranslatef(0, 0.25, i & 2 ? 2 : -2);
		} else {
			glTranslatef(i & 2 ? 2 : -2, 0.25, 0);
		}
		draw_box(0.5, 0.5, 0.5, 1.0);
		glPopMatrix();
	}

	col[0] = 1;
	col[1] = 1;
	col[2] = 0.4;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	draw_box(0.05, 1.2, 6, 1.0);
	draw_box(6, 1.2, 0.05, 1.0);

	/* draw tracking markers for the hands */
	for(i=0; i<2; i++) {
		if(goatvr_hand_active(i)) {
			float mcol[] = {0, 0, 0, 1};
			mcol[0] = 1 - i;
			mcol[3] = i;

			glPushMatrix();
			glTranslatef(0, goatvr_get_eye_height(), 0);
			glMultMatrixf(goatvr_hand_matrix(i));

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mcol);
			draw_box(0.02, 0.02, 0.02, 1.0);

			glPopMatrix();
		}
	}
}

static void draw_box(float xsz, float ysz, float zsz, float norm_sign)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(xsz * 0.5, ysz * 0.5, zsz * 0.5);

	if(norm_sign < 0.0) {
		glFrontFace(GL_CW);
	}

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1 * norm_sign);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, 1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, 1);
	glNormal3f(1 * norm_sign, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(1, -1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(1, 1, 1);
	glNormal3f(0, 0, -1 * norm_sign);
	glTexCoord2f(0, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(-1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(1, 1, -1);
	glNormal3f(-1 * norm_sign, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(-1, -1, 1);
	glTexCoord2f(1, 1); glVertex3f(-1, 1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, 1 * norm_sign, 0);
	glTexCoord2f(0.5, 0.5); glVertex3f(0, 1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
	glTexCoord2f(1, 0); glVertex3f(1, 1, 1);
	glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
	glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
	glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0, -1 * norm_sign, 0);
	glTexCoord2f(0.5, 0.5); glVertex3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
	glTexCoord2f(1, 1); glVertex3f(1, -1, 1);
	glTexCoord2f(0, 1); glVertex3f(-1, -1, 1);
	glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
	glEnd();

	glFrontFace(GL_CCW);
	glPopMatrix();
}

static void reshape(int x, int y)
{
	width = x;
	height = y;

	glViewport(0, 0, x, y);

	/* At least once in the beginning, and every time the output is resized,
	 * we should call goatvr_set_fb_size. Not strictly necessary for all VR
	 * modules, but good practice to avoid surprises.
	 * Last argument is the render buffer resolution scaling factor. Use lower
	 * values to trade off quality for speed, or higher values to trade off
	 * speed for quality.
	 */
	goatvr_set_fb_size(x, y, 1.0);
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

	case SDL_MOUSEMOTION:
		if(use_mouselook) {
			handle_relative_mouse_motion(ev->motion.xrel, ev->motion.yrel);
		}
		break;
	}
}

static void handle_key(int key, int press)
{
	if(key < 256) {
		keystate[key] = press;
	}
	if(press) {
		switch(key) {
		case SDLK_ESCAPE:
			done = 1;
			break;

		case ' ':
			printf("recenter\n");
			goatvr_recenter();
			break;

		case '`':
			use_mouselook = SDL_GetRelativeMouseMode() ? 0 : 1;
			printf("%s the mouse\n", use_mouselook ? "grabbing" : "releasing");
			SDL_SetRelativeMouseMode(use_mouselook);
			SDL_SetWindowGrab(win, use_mouselook);
			break;
		}
	}
}

static void handle_relative_mouse_motion(int dx, int dy)
{
	if(!dx && !dy) return;

	cam_theta += dx * 0.5;

	/* if the current VR module supports head-tracking, then it's best to
	 * leave camera elevation strictly to the user's head motion. Otherwise
	 * go into full mouselook mode.
	 */
	if(!goatvr_have_headtracking()) {
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
}

/* generate a chessboard texture with tiles colored (r0, g0, b0) and (r1, g1, b1) */
static unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1)
{
	unsigned int tex;
	unsigned char img[8 * 8 * 3];
	unsigned char *pix = img;

	for(int i=0; i<8; i++) {
		for(int j=0; j<8; j++) {
			int black = (i & 1) == (j & 1);
			pix[0] = (black ? r0 : r1) * 255;
			pix[1] = (black ? g0 : g1) * 255;
			pix[2] = (black ? b0 : b1) * 255;
			pix += 3;
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	return tex;
}
