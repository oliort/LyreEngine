#pragma once

#include "Control.h"

class Trigger : public Control{
protected:
	bool awaiting;
public:
	Trigger();
	virtual void press();
	virtual void process();
	virtual void release();
	virtual bool getAwaiting();
	virtual ~Trigger();
};
