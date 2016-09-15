#include "Button.h"

Button::Button()
	: Control(), activated(false){}

void Button::press(){
	activated = true;
}

void Button::process(){}

void Button::release(){
	activated = false;
}

bool Button::getAwaiting(){
	return activated;
}

Button::~Button(){}
