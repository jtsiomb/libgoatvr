#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "opengl.h"
#include "mesh.h"

static void distort_texcoords(float *tc, float aspect, float lens_center_offset, float scale,
		const float *dist_factors);
static float barrel_scale(float rad, const float *k);

static PFNGLGENBUFFERSARBPROC gl_gen_buffers;
static PFNGLDELETEBUFFERSARBPROC gl_delete_buffers;
static PFNGLBUFFERDATAARBPROC gl_buffer_data;
static PFNGLBINDBUFFERARBPROC gl_bind_buffer;

int vrimp_mesh_init(struct mesh *m)
{
	m->prim = GL_TRIANGLES;

	m->varr = 0;
	m->iarr = 0;
	m->num_verts = m->num_faces = 0;
	m->vbo = m->ibo = 0;

	if(!gl_gen_buffers) {
		gl_gen_buffers = (PFNGLGENBUFFERSARBPROC)vrimp_glfunc("glGenBuffersARB");
		gl_delete_buffers = (PFNGLDELETEBUFFERSARBPROC)vrimp_glfunc("glDeleteBuffersARB");
		gl_buffer_data = (PFNGLBUFFERDATAARBPROC)vrimp_glfunc("glBufferDataARB");
		gl_bind_buffer = (PFNGLBINDBUFFERARBPROC)vrimp_glfunc("glBindBufferARB");

		if(!(gl_gen_buffers && gl_delete_buffers && gl_buffer_data && gl_bind_buffer)) {
			fprintf(stderr, "Failed to load VBO functions\n");
			return -1;
		}
	}

	return 0;
}

void vrimp_mesh_destroy(struct mesh *m)
{
	free(m->varr);
	free(m->iarr);

	if(m->vbo) gl_delete_buffers(1, &m->vbo);
	if(m->ibo) gl_delete_buffers(1, &m->ibo);
}

void vrimp_mesh_draw(struct mesh *m)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	gl_bind_buffer(GL_ARRAY_BUFFER, m->vbo);
	glVertexPointer(3, GL_FLOAT, sizeof(struct vertex), 0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), (void*)offsetof(struct vertex, tx));

	gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	glDrawElements(GL_TRIANGLES, m->num_faces * 3, GL_UNSIGNED_INT, 0);

	gl_bind_buffer(GL_ARRAY_BUFFER, 0);
	gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

int vrimp_mesh_barrel_distortion(struct mesh *m, int usub, int vsub, float aspect,
		float lens_center_offset, float scale, const float *dist_factors,
		float tex_scale_x, float tex_scale_y)
{
	int i, j;
	int uverts, vverts;
	int num_verts, num_quads, num_tris;
	struct vertex *varr, *vptr;
	unsigned int *iarr, *iptr;
	float du, dv;

	uverts = usub + 1;
	vverts = vsub + 1;

	num_verts = uverts * vverts;
	num_quads = usub * vsub;
	num_tris = num_quads * 2;

	if(!(varr = malloc(num_verts * sizeof *varr))) {
		return -1;
	}
	if(!(iarr = malloc(num_tris * 3 * sizeof *iarr))) {
		free(varr);
		return -1;
	}

	du = 1.0 / (float)usub;
	dv = 1.0 / (float)vsub;

	vptr = varr;
	for(i=0; i<vverts; i++) {
		float v = (float)i * dv;
		float y = 2.0 * v - 1.0;

		for(j=0; j<uverts; j++) {
			float u = (float)j * du;
			float x = 2.0 * u - 1.0;
			float tc[2];
			tc[0] = u;
			tc[1] = v;

			distort_texcoords(tc, aspect, lens_center_offset, scale, dist_factors);

			vptr->x = x;
			vptr->y = y;
			vptr->z = 0;
			vptr->tx = tc[0] * tex_scale_x;
			vptr->ty = tc[1] * tex_scale_y;
			vptr++;
		}
	}

	iptr = iarr;
	for(i=0; i<vsub; i++) {
		for(j=0; j<usub; j++) {
			*iptr++ = i * uverts + j;
			*iptr++ = (i + 1) * uverts + j;
			*iptr++ = (i + 1) * uverts + (j + 1);

			*iptr++ = i * uverts + j;
			*iptr++ = (i + 1) * uverts + (j + 1);
			*iptr++ = i * uverts + (j + 1);
		}
	}

	gl_gen_buffers(1, &m->vbo);
	gl_gen_buffers(1, &m->ibo);
	gl_bind_buffer(GL_ARRAY_BUFFER, m->vbo);
	gl_buffer_data(GL_ARRAY_BUFFER, num_verts * sizeof *varr, varr, GL_STATIC_DRAW);
	gl_bind_buffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
	gl_buffer_data(GL_ELEMENT_ARRAY_BUFFER, num_tris * 3 * sizeof *iarr, iarr, GL_STATIC_DRAW);

	m->prim = GL_TRIANGLES;
	m->num_verts = num_verts;
	m->num_faces = num_tris;
	return 0;
}

static void distort_texcoords(float *tc, float aspect, float lens_center_offset, float scale,
		const float *dist_factors)
{
	/* map tc [0, 1] -> [-1, 1] */
	float ptx = tc[0] * 2.0 - 1.0;
	float pty = tc[1] * 2.0 - 1.0;
	float rad;

	ptx += lens_center_offset * 2.0;
	pty /= aspect;	/* correct for aspect ratio */

	rad = barrel_scale(ptx * ptx + pty * pty, dist_factors);
	ptx *= rad;	/* scale the point by the computer distortion radius */
	pty *= rad;

	ptx /= scale;
	pty /= scale;

	pty *= aspect;
	ptx -= lens_center_offset * 2.0;

	/* map back to range [0, 1] */
	tc[0] = ptx * 0.5 + 0.5;
	tc[1] = pty * 0.5 + 0.5;
}

static float barrel_scale(float rad, const float *k)
{
	float radsq = rad * rad;
	float radquad = radsq * radsq;
	return k[0] + k[1] * radsq + k[2] * radquad + k[3] * radquad * radsq;
}
