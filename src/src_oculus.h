#ifndef SRC_OCULUS_H_
#define SRC_OCULUS_H_

#include "source.h"

namespace goatvr {

class SrcOculus: public Source {
public:
	const char *get_name() const;

	int get_num_buttons() const;
	int get_num_axes() const;
	bool is_spatial() const;

	Vec3 get_position() const;
	Quat get_rotation() const;
	Mat4 get_matrix() const;
};

}	// namespace goatvr

#endif // SRC_OCULUS_H_
