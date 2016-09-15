#pragma once

#include <windows.h>

#include "KeyTranslator.h"

#include "ControlsApplier.h"

class KeyTranslatorDefault : public KeyTranslator{
public:
	KeyTranslatorDefault(ControlsApplier* applier);
	virtual void press(UINT key);
	virtual void release(UINT key);
	virtual ~KeyTranslatorDefault();
};
