#include "Trigger.h"

Trigger::Trigger()
	: Control(), awaiting(false){}

void Trigger::press() {
	awaiting = true;
}

void Trigger::process(){
	awaiting = false;
}

void Trigger::release(){}

bool Trigger::getAwaiting(){
	return awaiting;
}

Trigger::~Trigger(){}
