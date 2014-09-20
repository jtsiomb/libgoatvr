#ifndef MESH_H_
#define MESH_H_

struct vertex {
	float x, y, z;
	float tx, ty;
};

struct mesh {
	int prim;
	struct vertex *varr;
	unsigned int *iarr;
	int num_verts, num_faces;
	unsigned int vbo, ibo;
};

int vrimp_mesh_init(struct mesh *m);
void vrimp_mesh_destroy(struct mesh *m);

void vrimp_mesh_draw(struct mesh *m);

int vrimp_mesh_barrel_distortion(struct mesh *m, int usub, int vsub, float aspect,
		float lens_center_offset, float scale, const float *dist_factors,
		float tex_scale_x, float tex_scale_y);


#endif	/* MESH_H_ */
