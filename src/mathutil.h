#ifndef MATHUTIL_H_
#define MATHUTIL_H_

void rotation_matrix(const float *quat, float *matrix);
void translation_matrix(const float *vec, float *matrix);

void mult_matrix(float *dest, const float *m1, const float *m2);

void transpose_matrix(float *matrix);

void print_matrix(const float *matrix);

#endif	/* MATHUTIL_H_ */
