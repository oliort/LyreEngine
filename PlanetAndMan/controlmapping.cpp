#include "controlmapping.h"

using namespace ControlMapping;

CONTROL_TYPE ControlMapping::getControlType(CONTROL_PURPOSE purpose) {
	switch (purpose) {
	case CONTROL_CAM_TARG_UP:
	case CONTROL_CAM_TARG_DOWN:
	case CONTROL_CAM_TARG_LEFT:
	case CONTROL_CAM_TARG_RIGHT:
	case CONTROL_CAM_TARG_ROT_CW:
	case CONTROL_CAM_TARG_ROT_CCW:
	case CONTROL_CAM_TARG_ZIN:
	case CONTROL_CAM_TARG_ZOUT:
	case CONTROL_CAM_HEAD_UP:
	case CONTROL_CAM_HEAD_DOWN:
		return TYPE_BUTTON;
	case CONTROL_MAN_MOVE:
	case CONTROL_MAN_ROT_RIGHT:
	case CONTROL_MAN_ROT_LEFT:
		return TYPE_BUTTON;
	case CONTROL_SWITCH_CAMERA:
	case CONTROL_SWITCH_RENDER:
	case CONTROL_SWITCH_RASTERIZER:
		return TYPE_TRIGGER;
	default:
		return TYPE_UNKNOWN;
	}
}
