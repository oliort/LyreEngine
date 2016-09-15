#pragma once

#include "Control.h"
#include "controlmapping.h"

#define CAM_MOVE_SPEED			 0.01f
#define CAM_ROTATE_SPEED		 0.075f
#define CAM_ZOOM_SPEED			 1.5f

#define MAN_MOVE_SPEED			 0.0001f
#define MAN_ROTATE_SPEED		 0.075f

class PlanetAndMan;

class ControlsApplier{
protected:
	PlanetAndMan* pApplication;
	Control* controls[ControlMapping::CONTROL_PURPOSES_NUMBER];
public:
	ControlsApplier(PlanetAndMan* application);
	void press(int control);
	void release(int control);
	void processControls();
	~ControlsApplier();
};