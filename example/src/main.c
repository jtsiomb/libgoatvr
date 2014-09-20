#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "vr.h"

int init(void);
void cleanup(void);
void toggle_hmd_fullscreen(void);
void display(void);
void draw_scene(void);
void draw_box(float xsz, float ysz, float zsz, float norm_sign);
void update_rtarg(int width, int height);
int handle_event(SDL_Event *ev);
int key_event(int key, int state);
void reshape(int x, int y);
unsigned int next_pow2(unsigned int x);
void quat_to_matrix(const float *quat, float *mat);
unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1);

static SDL_Window *win;
static SDL_GLContext ctx;
static int win_width, win_height;

static unsigned int fbo, fb_tex, fb_depth;
static int fb_width, fb_height;
static int fb_tex_width, fb_tex_height;

static unsigned int chess_tex;


int main(int argc, char **argv)
{
	if(init() == -1) {
		return 1;
	}

	for(;;) {
		SDL_Event ev;
		while(SDL_PollEvent(&ev)) {
			if(handle_event(&ev) == -1) {
				goto done;
			}
		}
		display();
	}

done:
	cleanup();
	return 0;
}


int init(void)
{
	int x, y;
	unsigned int flags;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);

	x = y = SDL_WINDOWPOS_UNDEFINED;
	flags = SDL_WINDOW_OPENGL;
	if(!(win = SDL_CreateWindow("press 'f' to move to the HMD", x, y, 1280, 800, flags))) {
		fprintf(stderr, "failed to create window\n");
		return -1;
	}
	if(!(ctx = SDL_GL_CreateContext(win))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		return -1;
	}

	glewInit();

	if(vr_init() == -1) {
		return -1;
	}

	/* resize our window to match the HMD resolution */
	win_width = vr_get_opti(VR_OPT_DISPLAY_WIDTH);
	win_height = vr_get_opti(VR_OPT_DISPLAY_HEIGHT);
	if(!win_width || !win_height) {
		SDL_GetWindowSize(win, &win_width, &win_height);
	} else {
		SDL_SetWindowSize(win, win_width, win_height);
		SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	/* and create a single render target texture to encompass both eyes */
	fb_width = vr_get_opti(VR_OPT_LEYE_XRES) + vr_get_opti(VR_OPT_REYE_XRES);
	fb_height = vr_get_opti(VR_OPT_LEYE_YRES);	/* assuming both are the same */
	if(!fb_width || !fb_height) {
		fb_width = win_width;
		fb_height = win_height;
	}
	update_rtarg(fb_width, fb_height);

	/* set our render texture and its active area */
	vr_output_texture(fb_tex, 0, 0, (float)fb_width / (float)fb_tex_width, (float)fb_height / (float)fb_tex_height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_NORMALIZE);

	glClearColor(0.5, 0.1, 0.1, 1);

	chess_tex = gen_chess_tex(1.0, 0.7, 0.4, 0.4, 0.7, 1.0);
	return 0;
}

void cleanup(void)
{
	vr_shutdown();
	SDL_Quit();
}

void toggle_hmd_fullscreen(void)
{
	static int fullscr, prev_x, prev_y;
	fullscr = !fullscr;

	if(fullscr) {
		/* going fullscreen on the rift. save current window position, and move it
		 * to the rift's part of the desktop before going fullscreen
		 */
		SDL_GetWindowPosition(win, &prev_x, &prev_y);
		SDL_SetWindowPosition(win, vr_get_opti(VR_OPT_WIN_XOFFS), vr_get_opti(VR_OPT_WIN_YOFFS));
		SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		/* return to windowed mode and move the window back to its original position */
		SDL_SetWindowFullscreen(win, 0);
		SDL_SetWindowPosition(win, prev_x, prev_y);
	}
}

void display(void)
{
	int i;
	float proj_mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
	float view_mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

	/* start drawing onto our texture render target */
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* for each eye ... */
	for(i=0; i<2; i++) {
		vr_begin(i);

		/* -- viewport transformation --
		 * setup the viewport to draw in the left half of the framebuffer when we're
		 * rendering the left eye's view (0, 0, width/2, height), and in the right half
		 * of the framebuffer for the right eye's view (width/2, 0, width/2, height)
		 */
		glViewport(i == 0 ? 0 : fb_width / 2, 0, fb_width / 2, fb_height);

		glMatrixMode(GL_PROJECTION);
		/* -- projection transformation --
		 * we'll just have to use the projection matrix supplied by the oculus SDK for this eye
		 * note that libovr matrices are the transpose of what OpenGL expects, so we have to
		 * use glLoadTransposeMatrixf instead of glLoadMatrixf to load it.
		 */
		if(vr_proj_matrix(i, 0.5, 500.0, proj_mat)) {
			glLoadMatrixf(proj_mat);
		} else {
			glLoadIdentity();
			gluPerspective(50.0, (float)fb_width / 2.0 / (float)fb_height, 0.5, 500.0);
		}

		/* -- view/camera transformation --
		 * we need to construct a view matrix by combining all the information provided by the oculus
		 * SDK, about the position and orientation of the user's head in the world.
		 */
		glMatrixMode(GL_MODELVIEW);
		vr_view_matrix(i, view_mat);
		glLoadMatrixf(view_mat);
		/* move the camera to the eye level of the user */
		glTranslatef(0, -vr_get_optf(VR_OPT_EYE_HEIGHT), 0);

		/* finally draw the scene for this eye */
		draw_scene();

		vr_end();
	}

	/* after drawing both eyes into the texture render target, revert to drawing directly to the
	 * display, and we call ovrHmd_EndFrame, to let the Oculus SDK draw both images properly
	 * compensated for lens distortion and chromatic abberation onto the HMD screen.
	 */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, win_width, win_height);

	vr_swap_buffers();

	assert(glGetError() == GL_NO_ERROR);
}

void draw_scene(void)
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
}

void draw_box(float xsz, float ysz, float zsz, float norm_sign)
{
	glMatrixMode(GL_MODELVIEW);
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
}

/* update_rtarg creates (and/or resizes) the render target used to draw the two stero views */
void update_rtarg(int width, int height)
{
	if(!fbo) {
		/* if fbo does not exist, then nothing does... create every opengl object */
		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &fb_tex);
		glGenRenderbuffers(1, &fb_depth);

		glBindTexture(GL_TEXTURE_2D, fb_tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* calculate the next power of two in both dimensions and use that as a texture size */
	fb_tex_width = next_pow2(width);
	fb_tex_height = next_pow2(height);

	/* create and attach the texture that will be used as a color buffer */
	glBindTexture(GL_TEXTURE_2D, fb_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_tex_width, fb_tex_height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0);

	/* create and attach the renderbuffer that will serve as our z-buffer */
	glBindRenderbuffer(GL_RENDERBUFFER, fb_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fb_tex_width, fb_tex_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb_depth);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "incomplete framebuffer!\n");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	printf("created render target: %dx%d (texture size: %dx%d)\n", width, height, fb_tex_width, fb_tex_height);
}

int handle_event(SDL_Event *ev)
{
	switch(ev->type) {
	case SDL_QUIT:
		return -1;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
		if(key_event(ev->key.keysym.sym, ev->key.state == SDL_PRESSED) == -1) {
			return -1;
		}
		break;

	default:
		break;
	}

	return 0;
}

int key_event(int key, int state)
{
	if(state) {
		switch(key) {
		case 27:
			return -1;

		case ' ':
			/* allow the user to recenter by pressing space */
			vr_recenter();
			break;

		case 'f':
			/* press f to move the window to the HMD */
			toggle_hmd_fullscreen();
			break;

		default:
			break;
		}
	}
	return 0;
}

unsigned int next_pow2(unsigned int x)
{
	x -= 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

/* convert a quaternion to a rotation matrix */
void quat_to_matrix(const float *quat, float *mat)
{
	mat[0] = 1.0 - 2.0 * quat[1] * quat[1] - 2.0 * quat[2] * quat[2];
	mat[4] = 2.0 * quat[0] * quat[1] + 2.0 * quat[3] * quat[2];
	mat[8] = 2.0 * quat[2] * quat[0] - 2.0 * quat[3] * quat[1];
	mat[12] = 0.0f;

	mat[1] = 2.0 * quat[0] * quat[1] - 2.0 * quat[3] * quat[2];
	mat[5] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[2]*quat[2];
	mat[9] = 2.0 * quat[1] * quat[2] + 2.0 * quat[3] * quat[0];
	mat[13] = 0.0f;

	mat[2] = 2.0 * quat[2] * quat[0] + 2.0 * quat[3] * quat[1];
	mat[6] = 2.0 * quat[1] * quat[2] - 2.0 * quat[3] * quat[0];
	mat[10] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[1]*quat[1];
	mat[14] = 0.0f;

	mat[3] = mat[7] = mat[11] = 0.0f;
	mat[15] = 1.0f;
}

/* generate a chessboard texture with tiles colored (r0, g0, b0) and (r1, g1, b1) */
unsigned int gen_chess_tex(float r0, float g0, float b0, float r1, float g1, float b1)
{
	int i, j;
	unsigned int tex;
	unsigned char img[8 * 8 * 3];
	unsigned char *pix = img;

	for(i=0; i<8; i++) {
		for(j=0; j<8; j++) {
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
