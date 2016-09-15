#pragma once

#include "Control.h"

class Button : public Control{
protected:
	bool activated;
public:
	Button();
	virtual void press();
	virtual void process();
	virtual void release();
	virtual bool getAwaiting();
	virtual ~Button();
};