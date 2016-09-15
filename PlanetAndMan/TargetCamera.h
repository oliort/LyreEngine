#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>

#include "Camera.h"

class TargetCamera : public Camera {
protected:
	XMFLOAT3 target;
	float headingAngle;
public:
	TargetCamera(float x, float y, float z,
		float targ_x, float targ_y, float targ_z);
	void Look(float targ_x, float targ_y, float targ_z);
	void GoToTarget(float distance);
	void ApproachToTarget(float percentage);
	void RotateAroundTargetH(float angle);
	void RotateAroundTargetV(float angle);
	virtual void RotateUpAxis(float angle);
	virtual void RotateHeading(float angle);
	virtual XMFLOAT3 up();
	virtual XMFLOAT3 at();
	float targetDist();
	virtual ~TargetCamera();
};