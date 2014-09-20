#ifndef MATHUTIL_H_
#define MATHUTIL_H_

void vrimp_rotation_matrix(const float *quat, float *matrix);
void vrimp_translation_matrix(const float *vec, float *matrix);

void vrimp_mult_matrix(float *dest, const float *m1, const float *m2);

void vrimp_transpose_matrix(float *matrix);

void vrimp_print_matrix(const float *matrix);

#endif	/* MATHUTIL_H_ */
