#pragma once

#include <windows.h>

#include "ControlsApplier.h"

#define KEY_NUM_0		0x60
#define KEY_NUM_1		0x61
#define KEY_NUM_2		0x62
#define KEY_NUM_3		0x63
#define KEY_NUM_4		0x64
#define KEY_NUM_6		0x66
#define KEY_NUM_7		0x67
#define KEY_NUM_8		0x68
#define KEY_NUM_9		0x69
#define KEY_NUM_PLUS	VK_ADD
#define KEY_NUM_MINUS	VK_SUBTRACT
#define KEY_W			0x57
#define KEY_A			0x41
#define KEY_D			0x44
#define KEY_1			0x31
#define KEY_2			0x32

class KeyTranslator {
protected:
	ControlsApplier* pApplier;
public:
	KeyTranslator(ControlsApplier* applier);
	virtual void press(UINT key) = 0;
	virtual void release(UINT key) = 0;
	virtual ~KeyTranslator();
};
