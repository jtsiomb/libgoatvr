#include "opengl.h"
#include "goatvr_impl.h"
#include "modman.h"

using namespace goatvr;

extern "C" {

static bool in_vr;
// user-supplied framebuffer properties
static int cur_fbwidth, cur_fbheight;
static float cur_fbscale = 1.0f;

static unsigned int fbo;
static unsigned int zbuf;
static int fbo_width, fbo_height;

int goatvr_init()
{
	return -1;
}

void goatvr_shutdown()
{
	if(zbuf) {
	}
}

void goatvr_detect()
{
	printf("goatvr: module detection running for %d total modules ...\n", get_num_modules());
	detect();

	printf("goatvr: detected %d usable modules:\n", get_num_usable_modules());
	for(int i=0; i<get_num_modules(); i++) {
		Module *m = get_module(i);
		if(m->usable()) {
			printf("  %s", m->get_name());
			if(m->get_type() == MODULE_RENDERING) {
				printf(" (rendering module)\n");
			} else {
				putchar('\n');
			}
		}
	}
}

void goatvr_startvr()
{
	if(in_vr) return;

	// make sure we have at least one rendering module
	if(!render_module) {
		fprintf(stderr, "goatvr: can't enter VR mode, no usable rendering module found\n");
		return;
	}

	start();
	in_vr = true;
}

void goatvr_stopvr()
{
	if(in_vr) stop();
}

int goatvr_invr()
{
	return in_vr ? 1 : 0;
}

// ---- rendering ----

void goatvr_set_fb_size(int width, int height, float scale)
{
	if(render_module) {
		render_module->set_fbsize(width, height, scale);
	}
	cur_fbwidth = width;
	cur_fbheight = height;
	cur_fbscale = scale;
}

float goatvr_get_fb_scale()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->fbscale;
	}
	return cur_fbscale;
}

int goatvr_get_fb_width()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->width;
	}
	return cur_fbwidth;
}

int goatvr_get_fb_height()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->height;
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_width(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_width[eye];
	}
	return cur_fbwidth / 2;
}

int goatvr_get_fb_eye_height(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_height[eye];
	}
	return cur_fbheight;
}

int goatvr_get_fb_eye_xoffset(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_xoffs[eye];
	}
	return eye == GOATVR_LEFT ? 0 : cur_fbwidth / 2;
}

int goatvr_get_fb_eye_yoffset(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->eye_yoffs[eye];
	}
	return 0;
}

unsigned int goatvr_get_fb_texture()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		return rtex->tex;
	}
	return 0;
}

int goatvr_get_fb_texture_width()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		rtex->tex_width;
	}
	return next_pow2(cur_fbwidth);
}

int goatvr_get_fb_texture_height()
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		rtex->tex_height;
	}
	return next_pow2(cur_fbheight);
}

unsigned int goatvr_get_fbo(void)
{
	if(!render_module) {
		return 0;
	}

	RenderTexture *rtex = render_module->get_render_texture();
	if(!rtex) {
		return 0;
	}

	if(!fbo) {
		glGenFramebuffers(1, &fbo);
		glGenRenderbuffers(1, &zbuf);
	}

	// resize fbo if necessary (or if it's the first time)
	if(fbo_width != rtex->width || fbo_height != rtex->height) {
		fbo_width = rtex->width;
		fbo_height = rtex->height;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtex->tex, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, zbuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_width, fbo_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuf);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "goatvr: incomplete framebuffer!\n");
			return 0;
		}
	}
	return fbo;
}

void goatvr_viewport(int eye)
{
	if(render_module) {
		RenderTexture *rtex = render_module->get_render_texture();
		glViewport(rtex->eye_xoffs[eye], rtex->eye_yoffs[eye], rtex->eye_width[eye], rtex->eye_height[eye]);
	} else {
		glViewport(eye == 0 ? 0 : cur_fbwidth / 2, 0, cur_fbwidth / 2, cur_fbheight);
	}
}

static float ident_mat[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

float *goatvr_view_matrix(int eye)
{
	if(render_module) {
		Mat4 vmat = render_module->get_view_matrix(eye);
		return vmat[0];
	}
	return ident_mat;
}

float *goatvr_projection_matrix(int eye, float znear, float zfar)
{
	if(render_module) {
		Mat4 pmat = render_module->get_proj_matrix(eye, znear, zfar);
		return pmat[0];
	}
	return ident_mat;
}

static bool in_drawing;

void goatvr_draw_eye(int eye)
{
	if(!render_module) return;

	if(!in_drawing) {
		/* NOTE: we must call goatvr_get_fbo instead of accessing fbo directly here
		 * to make sure we have an up to date render texture and framebuffer object
		 */
		glBindFramebuffer(GL_FRAMEBUFFER, goatvr_get_fbo());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render_module->draw_start();
		in_drawing = true;
	}

	goatvr_viewport(eye);
	render_module->draw_eye(eye);
}

void goatvr_draw_done()
{
	if(!in_drawing || !render_module) return;

	in_drawing = false;

	render_module->draw_done();
}

}	// extern "C"


unsigned int goatvr::next_pow2(unsigned int x)
{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
