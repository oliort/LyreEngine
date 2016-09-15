#pragma once
#include "Camera.h"
class SurfaceCamera : public Camera {
protected:
	XMFLOAT3 target;
public:
	SurfaceCamera(float x, float y, float z,
		float targ_x, float targ_y, float targ_z);
	void Look(float targ_x, float targ_y, float targ_z);
	void GoToTarget(float distance);
	void ApproachToTarget(float percentage);
	void RotateAroundTargetH(float angle);
	void RotateAroundTargetV(float angle);
	virtual void RotateUpAxis(float angle);
	float targetX();
	float targetY();
	float targetZ();
	float targetDist();
	virtual ~SurfaceCamera();
};

