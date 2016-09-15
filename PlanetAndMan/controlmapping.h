#pragma once

namespace ControlMapping {

	enum CONTROL_PURPOSE {
		///target camera
		CONTROL_CAM_TARG_UP,
		CONTROL_CAM_TARG_DOWN,
		CONTROL_CAM_TARG_LEFT,
		CONTROL_CAM_TARG_RIGHT,
		CONTROL_CAM_TARG_ROT_CW,
		CONTROL_CAM_TARG_ROT_CCW,
		CONTROL_CAM_TARG_ZIN,
		CONTROL_CAM_TARG_ZOUT,
		CONTROL_CAM_HEAD_UP,
		CONTROL_CAM_HEAD_DOWN,
		///character
		CONTROL_MAN_MOVE,
		CONTROL_MAN_ROT_RIGHT,
		CONTROL_MAN_ROT_LEFT,
		///switching camera and rendering
		CONTROL_SWITCH_CAMERA,
		CONTROL_SWITCH_RENDER,
		CONTROL_SWITCH_RASTERIZER,
		///last enumeration element provides number of elements
		///Caution!!! The elements must be sequential and start with 0 (as by default)
		CONTROL_PURPOSES_NUMBER
	};

	enum CONTROL_TYPE {
		TYPE_UNKNOWN,
		TYPE_BUTTON,
		TYPE_TRIGGER
	};

	extern CONTROL_TYPE getControlType(CONTROL_PURPOSE purpose);
}