#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>

#define PI 3.14159265359f

class Camera {
protected:
	XMFLOAT3 position;
	XMFLOAT3 up_axis;
	float field_of_view;
public:
	Camera(float x, float y, float z);
	void GoTo(float x, float y, float z);
	void Move(float x, float y, float z);
	virtual void RotateUpAxis(float angle) = 0;
	void setFOV(float value);
	XMFLOAT3 pos();
	virtual XMFLOAT3 up();
	virtual XMFLOAT3 at() = 0;
	float FOV();
	virtual ~Camera();
};