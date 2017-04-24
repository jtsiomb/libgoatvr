#include "src_oculus.h"

using namespace goatvr;

const char *SrcOculus::get_name() const
{
	return "oculus";
}

int SrcOculus::get_num_buttons() const
{
	return 0;	// TODO
}

int SrcOculus::get_num_axes() const
{
	return 0;	// TODO
}

bool SrcOculus::is_spatial() const
{
	return true;
}


Vec3 SrcOculus::get_position() const
{
	return Vec3(0, 0, 0);	// TODO
}

Quat SrcOculus::get_rotation() const
{
	return Quat::identity;	// TODO
}

Mat4 SrcOculus::get_matrix() const
{
	return Mat4::identity;	// TODO
}
