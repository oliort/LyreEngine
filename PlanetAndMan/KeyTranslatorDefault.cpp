#include "KeyTranslatorDefault.h"

#include "controlmapping.h"

using namespace ControlMapping;

KeyTranslatorDefault::KeyTranslatorDefault(ControlsApplier* applier)
	: KeyTranslator(applier) {}

void KeyTranslatorDefault::press(UINT key) {
	switch (key) {
		//target camera
	case KEY_NUM_8: pApplier->press(CONTROL_CAM_TARG_UP); break;
	case KEY_NUM_2: pApplier->press(CONTROL_CAM_TARG_DOWN); break;
	case KEY_NUM_4: pApplier->press(CONTROL_CAM_TARG_LEFT); break;
	case KEY_NUM_6: pApplier->press(CONTROL_CAM_TARG_RIGHT); break;
	case KEY_NUM_9: pApplier->press(CONTROL_CAM_TARG_ROT_CW); break;
	case KEY_NUM_7: pApplier->press(CONTROL_CAM_TARG_ROT_CCW); break;
	case KEY_NUM_PLUS: pApplier->press(CONTROL_CAM_TARG_ZIN); break;
	case KEY_NUM_MINUS: pApplier->press(CONTROL_CAM_TARG_ZOUT); break;
	case KEY_NUM_1: pApplier->press(CONTROL_CAM_HEAD_UP); break;
	case KEY_NUM_3: pApplier->press(CONTROL_CAM_HEAD_DOWN); break;
		//character
	case KEY_W: pApplier->press(CONTROL_MAN_MOVE); break;
	case KEY_A: pApplier->press(CONTROL_MAN_ROT_LEFT); break;
	case KEY_D: pApplier->press(CONTROL_MAN_ROT_RIGHT); break;
		//switching camera and rendering
	case KEY_1: pApplier->press(CONTROL_SWITCH_CAMERA); break;
	case KEY_2: pApplier->press(CONTROL_SWITCH_RENDER); break;
	case KEY_NUM_0: pApplier->press(CONTROL_SWITCH_RASTERIZER); break;
	default: break;
	}
}

void KeyTranslatorDefault::release(UINT key) {
	switch (key) {
		//target camera
	case KEY_NUM_8: pApplier->release(CONTROL_CAM_TARG_UP); break;
	case KEY_NUM_2: pApplier->release(CONTROL_CAM_TARG_DOWN); break;
	case KEY_NUM_4: pApplier->release(CONTROL_CAM_TARG_LEFT); break;
	case KEY_NUM_6: pApplier->release(CONTROL_CAM_TARG_RIGHT); break;
	case KEY_NUM_9: pApplier->release(CONTROL_CAM_TARG_ROT_CW); break;
	case KEY_NUM_7: pApplier->release(CONTROL_CAM_TARG_ROT_CCW); break;
	case KEY_NUM_PLUS: pApplier->release(CONTROL_CAM_TARG_ZIN); break;
	case KEY_NUM_MINUS: pApplier->release(CONTROL_CAM_TARG_ZOUT); break;
	case KEY_NUM_1: pApplier->release(CONTROL_CAM_HEAD_UP); break;
	case KEY_NUM_3: pApplier->release(CONTROL_CAM_HEAD_DOWN); break;
		//character
	case KEY_W: pApplier->release(CONTROL_MAN_MOVE); break;
	case KEY_A: pApplier->release(CONTROL_MAN_ROT_LEFT); break;
	case KEY_D: pApplier->release(CONTROL_MAN_ROT_RIGHT); break;
		//switching camera and rendering
	case KEY_1: pApplier->release(CONTROL_SWITCH_CAMERA); break;
	case KEY_2: pApplier->release(CONTROL_SWITCH_RENDER); break;
	case KEY_NUM_0: pApplier->release(CONTROL_SWITCH_RASTERIZER); break;
	default: break;
	}
}

KeyTranslatorDefault::~KeyTranslatorDefault(){}
