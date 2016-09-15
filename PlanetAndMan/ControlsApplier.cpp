#include "ControlsApplier.h"

#include "Button.h"
#include "Trigger.h"
#include "PlanetAndMan.h"

#include "TargetCamera.h"
#include "Character.h"

using namespace ControlMapping;

ControlsApplier::ControlsApplier(PlanetAndMan* application)
	: pApplication(application){
	for (int i = 0; i < CONTROL_PURPOSES_NUMBER; i++) {
		switch (getControlType((CONTROL_PURPOSE)i)) {
		case TYPE_BUTTON:
			controls[i] = new Button(); break;
		case TYPE_TRIGGER:
			controls[i] = new Trigger(); break;
		}
	}
}

void ControlsApplier::press(int control){
	controls[control]->press();
}

void ControlsApplier::release(int control){
	controls[control]->release();
}

void ControlsApplier::processControls(){
	//target camera
	if (controls[CONTROL_CAM_TARG_UP]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateAroundTargetV(CAM_MOVE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_DOWN]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateAroundTargetV(-CAM_MOVE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_LEFT]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateAroundTargetH(CAM_MOVE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_RIGHT]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateAroundTargetH(-CAM_MOVE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_ROT_CW]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateUpAxis(-CAM_ROTATE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_ROT_CCW]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateUpAxis(CAM_ROTATE_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_ZIN]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->ApproachToTarget(CAM_ZOOM_SPEED);
	}
	if (controls[CONTROL_CAM_TARG_ZOUT]->getAwaiting()) {
		if (TargetCamera *camera = 
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->ApproachToTarget(-CAM_ZOOM_SPEED);
	}
	if (controls[CONTROL_CAM_HEAD_UP]->getAwaiting()) {
		if (TargetCamera *camera =
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateHeading(CAM_ROTATE_SPEED);
	}
	if (controls[CONTROL_CAM_HEAD_DOWN]->getAwaiting()) {
		if (TargetCamera *camera =
			dynamic_cast<TargetCamera*>(pApplication->getCamera()))
			camera->RotateHeading(-CAM_ROTATE_SPEED);
	}
	//character
	if (controls[CONTROL_MAN_MOVE]->getAwaiting()) {
		if (Character *man = pApplication->getMan())
			man->tryMoveForward(MAN_MOVE_SPEED);
	}
	if (controls[CONTROL_MAN_ROT_LEFT]->getAwaiting()) {
		if (Character *man = pApplication->getMan())
			man->rotateLeft(MAN_ROTATE_SPEED);
	}
	if (controls[CONTROL_MAN_ROT_RIGHT]->getAwaiting()) {
		if (Character *man = pApplication->getMan())
			man->rotateRight(MAN_ROTATE_SPEED);
	}
	//switching camera and rendering
	if (controls[CONTROL_SWITCH_CAMERA]->getAwaiting()) {
		pApplication->switchCamera();
	}
	if (controls[CONTROL_SWITCH_RENDER]->getAwaiting()) {
		pApplication->switchRender();
	}
	if (controls[CONTROL_SWITCH_RASTERIZER]->getAwaiting()) {
		pApplication->switchRasterizer();
	}
	for (int i = 0; i < CONTROL_PURPOSES_NUMBER; i++) {
		controls[i]->process();
	}
}

ControlsApplier::~ControlsApplier(){
	for (int i = 0; i < ControlMapping::CONTROL_PURPOSES_NUMBER; i++) {
		delete controls[i];
	}
}
