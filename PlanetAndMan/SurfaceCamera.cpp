#include "SurfaceCamera.h"

SurfaceCamera::SurfaceCamera(float x, float y, float z, float targ_x, float targ_y, float targ_z)
	:Camera(x, y, z), target{ targ_x, targ_y, targ_z } {}

void SurfaceCamera::Look(float targ_x, float targ_y, float targ_z) {
	target = { targ_x,targ_y,targ_z };
}

void SurfaceCamera::GoToTarget(float distance) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) +
		XMVector3Normalize(XMLoadFloat3(&target)
			- XMLoadFloat3(&position)) * distance);
}

void SurfaceCamera::ApproachToTarget(float perñentage) {
	//SAME AS : position += (target - position) * p
	XMStoreFloat3(&position,
		XMLoadFloat3(&position) * ((100.f - perñentage) / 100.f) +
		XMLoadFloat3(&target) * (perñentage / 100.f));
}

void SurfaceCamera::RotateAroundTargetH(float angle) {
	XMStoreFloat3(&position, XMLoadFloat3(&target) +
		XMVector3Rotate(XMLoadFloat3(&position) - XMLoadFloat3(&target),
			XMQuaternionRotationNormal(XMLoadFloat3(&up_axis), angle)));
}

void SurfaceCamera::RotateAroundTargetV(float angle) {
	XMVECTOR direction = XMLoadFloat3(&position) - XMLoadFloat3(&target);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&up_axis));
	direction = XMVector3Rotate(direction, XMQuaternionRotationAxis(right_axis, angle));
	XMStoreFloat3(&position, XMLoadFloat3(&target) + direction);
	XMStoreFloat3(&up_axis, XMVector3Normalize(XMVector3Cross(right_axis, direction)));
}

void SurfaceCamera::RotateUpAxis(float angle) {
	XMStoreFloat3(&up_axis, XMVector3Rotate(XMLoadFloat3(&up_axis),
		XMQuaternionRotationAxis(XMLoadFloat3(&target) - XMLoadFloat3(&position), angle)));
}

float SurfaceCamera::targetX() { return target.x; }
float SurfaceCamera::targetY() { return target.y; }
float SurfaceCamera::targetZ() { return target.z; }

float SurfaceCamera::targetDist() {
	float len;
	XMStoreFloat(&len, XMVector3Length(XMLoadFloat3(&target) - XMLoadFloat3(&position)));
	return len;
}

SurfaceCamera::~SurfaceCamera() {}

