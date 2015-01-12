#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "opt.h"
#include "rbtree.h"

static void opt_del_func(struct rbnode *opt, void *cls)
{
	free(opt);
}

void *create_options(void)
{
	struct rbtree *db = rb_create(RB_KEY_STRING);
	rb_set_delete_func(db, opt_del_func, 0);
	return db;
}

void destroy_options(void *optdb)
{
	rb_destroy(optdb);
}

void set_option_int(void *optdb, const char *key, int val)
{
	int i;
	struct option *opt = malloc(sizeof *opt);
	if(!opt) {
		fprintf(stderr, "failed to set option: %s: %s\n", key, strerror(errno));
		return;
	}
	opt->type = OTYPE_INT;
	opt->ival = val;
	opt->fval = (float)val;
	for(i=0; i<4; i++) {
		opt->vval[i] = opt->fval;
	}

	if(rb_insert(optdb, (void*)key, opt) == -1) {
		fprintf(stderr, "failed to set option: %s\n", key);
	}
}

void set_option_float(void *optdb, const char *key, float val)
{
	int i;
	struct option *opt = malloc(sizeof *opt);
	if(!opt) {
		fprintf(stderr, "failed to set option: %s: %s\n", key, strerror(errno));
		return;
	}
	opt->type = OTYPE_FLOAT;
	opt->fval = val;
	opt->ival = (int)val;
	for(i=0; i<4; i++) {
		opt->vval[i] = val;
	}

	if(rb_insert(optdb, (void*)key, opt) == -1) {
		fprintf(stderr, "failed to set option: %s\n", key);
	}
}

void set_option_vec(void *optdb, const char *key, float *val)
{
	int i;
	struct option *opt = malloc(sizeof *opt);
	if(!opt) {
		fprintf(stderr, "failed to set option: %s: %s\n", key, strerror(errno));
		return;
	}
	opt->type = OTYPE_VEC;
	for(i=0; i<4; i++) {
		opt->vval[i] = val[i];
	}
	opt->fval = val[0];
	opt->ival = (int)val[0];

	if(rb_insert(optdb, (void*)key, opt) == -1) {
		fprintf(stderr, "failed to set option: %s\n", key);
	}
}

void set_option_vec3f(void *optdb, const char *key, float x, float y, float z)
{
	float vec[4];
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	vec[3] = 1.0f;
	set_option_vec(optdb, key, vec);
}

void set_option_vec4f(void *optdb, const char *key, float x, float y, float z, float w)
{
	float vec[4];
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	vec[3] = w;
	set_option_vec(optdb, key, vec);
}

int get_option_int(void *optdb, const char *key, int *val)
{
	struct option *opt = rb_find(optdb, (void*)key);
	if(!opt) {
		*val = 0;
		return -1;
	}
	*val = opt->ival;
	return 0;
}

int get_option_float(void *optdb, const char *key, float *val)
{
	struct option *opt = rb_find(optdb, (void*)key);
	if(!opt) {
		*val = 0.0f;
		return -1;
	}
	*val = opt->fval;
	return 0;
}

int get_option_vec(void *optdb, const char *key, float *val)
{
	int i;
	struct option *opt = rb_find(optdb, (void*)key);
	if(!opt) {
		val[0] = val[1] = val[2] = val[3] = 0.0f;
		return -1;
	}

	for(i=0; i<4; i++) {
		val[i] = opt->vval[i];
	}
	return 0;
}
