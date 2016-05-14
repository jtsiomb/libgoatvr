#ifndef MODULE_H_
#define MODULE_H_

#include <gmath/gmath.h>
#include "rtex.h"

namespace goatvr {

// rendering modules can only be enabled one at a time
enum ModuleType { MODULE_RENDERING, MODULE_OTHER };

class Module {
protected:
	bool avail, act;

public:
	Module();
	virtual ~Module();

	virtual bool init();
	virtual void destroy();

	virtual ModuleType get_type() const = 0;
	virtual const char *get_name() const = 0;

	virtual void detect() = 0;
	virtual bool usable() const;

	virtual void activate();
	virtual void deactivate();
	virtual bool active() const;

	virtual void start();
	virtual void stop();

	virtual void update();

	// rendering ops are only valid on rendering modules
	virtual void set_fbsize(int width, int height);
	virtual RenderTexture *get_render_texture(float fbscale);

	virtual void draw_eye(int eye);
	virtual void draw_done();

	virtual Mat4 get_view_matrix(int eye);
	virtual Mat4 get_proj_matrix(int eye);

	void print_info(const char *fmt, ...) const;
	void print_error(const char *fmt, ...) const;
};

}	// namespace goatvr

#endif	/* MODULE_H_ */
