#ifndef MODULE_H_
#define MODULE_H_

namespace goatvr {

class Module {
protected:
	bool avail, act;

public:
	Module();
	virtual ~Module();

	virtual const char *get_name() const = 0;

	virtual void detect() = 0;
	virtual bool usable() const;

	virtual void activate();
	virtual void deactivate();
	virtual bool active() const;

	virtual void start() = 0;
	virtual void stop() = 0;
};

}

#endif	/* MODULE_H_ */
