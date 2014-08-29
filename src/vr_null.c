#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "vr_impl.h"

static unsigned int eye_tex[2];
static float tex_umin[2], tex_umax[2];
static float tex_vmin[2], tex_vmax[2];

static int init(void)
{
	return 0;
}

static int present(void)
{
	int i;

	glPushAttrib(GL_ENABLE_BIT | GL_TRANSFORM_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	for(i=0; i<2; i++) {
		float x0 = i == 0 ? -1 : 0;
		float x1 = i == 0 ? 0 : 1;

		glBindTexture(GL_TEXTURE_2D, eye_tex[i]);

		glBegin(GL_QUADS);
		glTexCoord2f(tex_umin[i], tex_vmin[i]);
		glVertex2f(x0, -1);
		glTexCoord2f(tex_umax[i], tex_vmin[i]);
		glVertex2f(x1, -1);
		glTexCoord2f(tex_umax[i], tex_vmax[i]);
		glVertex2f(x1, 1);
		glTexCoord2f(tex_umin[i], tex_vmax[i]);
		glVertex2f(x0, 1);
		glEnd();
	}

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
	return 0;
}

static void set_eye_texture(int eye, unsigned int tex, float umin, float vmin, float umax, float vmax)
{
	eye_tex[eye] = tex;
	tex_umin[eye] = umin;
	tex_umax[eye] = umax;
	tex_vmin[eye] = vmin;
	tex_vmax[eye] = vmax;
}

struct vr_module *vr_module_null(void)
{
	static struct vr_module m;

	if(!m.init) {
		m.name = "null";
		m.init = init;
		m.set_eye_texture = set_eye_texture;
		m.present = present;
	}
	return &m;
}
