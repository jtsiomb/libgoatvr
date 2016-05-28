#ifndef MOD_SBS_H_
#define MOD_SBS_H_

#include "module.h"

namespace goatvr {

class ModuleSBS : public Module {
protected:
	bool started;
	int win_width, win_height;

	float ipd;

	goatvr_origin_mode origin_mode;

public:
	ModuleSBS();
	~ModuleSBS();

	bool init();
	void destroy();

	ModuleType get_type() const;
	const char *get_name() const;

	bool detect();
	void start();

	void set_origin_mode(goatvr_origin_mode mode);

	void set_fbsize(int width, int height, float fbscale);

	Mat4 get_view_matrix(int eye);
	Mat4 get_proj_matrix(int eye, float znear, float zfar);
};

}	// namespace goatvr

#endif	// MOD_SBS_H_
