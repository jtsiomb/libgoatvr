#include <stdio.h>
#include <string.h>
#include "mathutil.h"

#define M(i, j)	((i) * 4 + (j))

static const float idmat[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

void vrimp_rotation_matrix(const float *quat, float *matrix)
{
	matrix[0] = 1.0 - 2.0 * quat[1] * quat[1] - 2.0 * quat[2] * quat[2];
	matrix[1] = 2.0 * quat[0] * quat[1] + 2.0 * quat[3] * quat[2];
	matrix[2] = 2.0 * quat[2] * quat[0] - 2.0 * quat[3] * quat[1];
	matrix[3] = 0.0f;

	matrix[4] = 2.0 * quat[0] * quat[1] - 2.0 * quat[3] * quat[2];
	matrix[5] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[2]*quat[2];
	matrix[6] = 2.0 * quat[1] * quat[2] + 2.0 * quat[3] * quat[0];
	matrix[7] = 0.0f;

	matrix[8] = 2.0 * quat[2] * quat[0] + 2.0 * quat[3] * quat[1];
	matrix[9] = 2.0 * quat[1] * quat[2] - 2.0 * quat[3] * quat[0];
	matrix[10] = 1.0 - 2.0 * quat[0]*quat[0] - 2.0 * quat[1]*quat[1];
	matrix[11] = 0.0f;

	matrix[12] = matrix[13] = matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	vrimp_transpose_matrix(matrix);
}

void vrimp_translation_matrix(const float *vec, float *matrix)
{
	memcpy(matrix, idmat, sizeof idmat);
	matrix[12] = vec[0];
	matrix[13] = vec[1];
	matrix[14] = vec[2];
}

void vrimp_mult_matrix(float *dest, const float *m1, const float *m2)
{
	int i, j;
	float tmp[16];

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			tmp[M(i, j)] = m1[M(i, 0)] * m2[M(0, j)] + m1[M(i, 1)] * m2[M(1, j)] +
				m1[M(i, 2)] * m2[M(2, j)] + m1[M(i, 3)] * m2[M(3, j)];
		}
	}
	memcpy(dest, tmp, sizeof tmp);
}

void vrimp_transpose_matrix(float *matrix)
{
	int i, j;
	float tmp[16];

	for(i=0; i<4; i++) {
		for(j=0; j<4; j++) {
			tmp[M(i, j)] = matrix[M(j, i)];
		}
	}
	memcpy(matrix, tmp, sizeof tmp);
}

void vrimp_print_matrix(const float *matrix)
{
	int i, j;

	for(i=0; i<4; i++) {
		putchar('[');
		for(j=0; j<4; j++) {
			printf("%6.3f ", matrix[M(i, j)]);
		}
		printf("]\n");
	}
}
