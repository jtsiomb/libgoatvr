#ifndef OPT_H_
#define OPT_H_

enum opt_type { OTYPE_INT, OTYPE_FLOAT };

struct option {
	enum opt_type type;
	int ival;
	float fval;
};

void *create_options(void);
void destroy_options(void *optdb);

void set_option_int(void *optdb, const char *key, int val);
void set_option_float(void *optdb, const char *key, float val);

int get_option_int(void *optdb, const char *key, int *val);
int get_option_float(void *optdb, const char *key, float *val);

#endif	/* OPT_H_ */
