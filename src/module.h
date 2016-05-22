#ifndef MODULE_H_
#define MODULE_H_

#include <gmath/gmath.h>
#include "goatvr_impl.h"
#include "rtex.h"

// use this in mod_whatever.cc to register each module: REG_MODULE(whatever, ModuleWhatever)
#define REG_MODULE(name, class_name) \
	namespace goatvr { \
		void add_module(Module*); \
		void register_mod_##name() { add_module(new class_name); } \
	}

// use this in mod_whatever.cc when you DON'T want to register that module: NOREG_MODULE(whatever)
#define NOREG_MODULE(name) \
	namespace goatvr { \
		void register_mod_##name() {} \
	}

namespace goatvr {

// rendering modules can only be enabled one at a time
enum ModuleType { MODULE_RENDERING, MODULE_OTHER };

class Module {
protected:
	int prio;
	bool avail, act;

public:
	Module();
	virtual ~Module();

	virtual bool init();
	virtual void destroy();

	virtual ModuleType get_type() const = 0;
	virtual const char *get_name() const = 0;

	virtual void set_priority(int p);
	virtual int get_priority() const;

	virtual bool detect();
	virtual bool usable() const;

	virtual void activate();
	virtual void deactivate();
	virtual bool active() const;

	virtual void start();
	virtual void stop();

	virtual void set_origin_mode(goatvr_origin_mode mode);

	virtual void update();

	// rendering ops are only valid on rendering modules
	virtual void set_fbsize(int width, int height, float fbscale);
	virtual RenderTexture *get_render_texture();

	virtual void draw_start();
	virtual void draw_eye(int eye);
	virtual void draw_done();
	virtual void draw_mirror();

	virtual Mat4 get_view_matrix(int eye);
	virtual Mat4 get_proj_matrix(int eye, float znear, float zfar);

	void print_info(const char *fmt, ...) const;
	void print_error(const char *fmt, ...) const;
};

}	// namespace goatvr

#endif	/* MODULE_H_ */
