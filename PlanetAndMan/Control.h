#pragma once

class Control
{
public:
	Control();
	virtual void press() = 0;
	virtual void process() = 0;
	virtual void release() = 0;
	virtual bool getAwaiting() = 0;
	virtual ~Control();
};

