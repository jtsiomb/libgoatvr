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
	struct option *opt = malloc(sizeof *opt);
	if(!opt) {
		fprintf(stderr, "failed to set option: %s: %s\n", key, strerror(errno));
		return;
	}
	opt->type = OTYPE_INT;
	opt->ival = val;
	opt->fval = (float)val;

	if(rb_insert(optdb, (void*)key, opt) == -1) {
		fprintf(stderr, "failed to set option: %s\n", key);
	}
}

void set_option_float(void *optdb, const char *key, float val)
{
	struct option *opt = malloc(sizeof *opt);
	if(!opt) {
		fprintf(stderr, "failed to set option: %s: %s\n", key, strerror(errno));
		return;
	}
	opt->type = OTYPE_FLOAT;
	opt->fval = val;
	opt->ival = (int)val;

	if(rb_insert(optdb, (void*)key, opt) == -1) {
		fprintf(stderr, "failed to set option: %s\n", key);
	}
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
