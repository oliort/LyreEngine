#include "TargetCamera.h"

#include "PlanetAndMan.h"

TargetCamera::TargetCamera(float x, float y, float z, float targ_x, float targ_y, float targ_z)
	:Camera(x, y, z), target{ targ_x, targ_y, targ_z }, headingAngle(0.f) {}

void TargetCamera::Look(float targ_x, float targ_y, float targ_z){
	target = { targ_x,targ_y,targ_z };
}

void TargetCamera::GoToTarget(float distance) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) + 
		XMVector3Normalize(XMLoadFloat3(&target)
			- XMLoadFloat3(&position)) * distance);
}

void TargetCamera::ApproachToTarget(float percentage){
	//SAME AS : position += (target - position) * p4
	XMVECTOR direction =  XMLoadFloat3(&target) - XMLoadFloat3(&position);
	float moveLength = (XMVectorGetX(XMVector3Length(direction)) - PLANET_RAD) * percentage / 100.f;
	XMStoreFloat3(&position, XMLoadFloat3(&position) + 
		(moveLength > 0 ? 
			XMVector3ClampLength(direction, moveLength, moveLength) :
			-XMVector3ClampLength(direction, -moveLength, -moveLength)));
}

void TargetCamera::RotateAroundTargetH(float angle){
	XMStoreFloat3(&position, XMLoadFloat3(&target) +
		XMVector3Rotate(XMLoadFloat3(&position) - XMLoadFloat3(&target),
		XMQuaternionRotationNormal(XMLoadFloat3(&up_axis), angle)));
}

void TargetCamera::RotateAroundTargetV(float angle) {
	XMVECTOR direction = XMLoadFloat3(&position) - XMLoadFloat3(&target);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&up_axis));
	direction = XMVector3Rotate(direction, XMQuaternionRotationAxis(right_axis, angle));
	XMStoreFloat3(&position, XMLoadFloat3(&target) + direction);
	XMStoreFloat3(&up_axis, XMVector3Normalize(XMVector3Cross(right_axis, direction)));
}

void TargetCamera::RotateUpAxis(float angle) {
	XMStoreFloat3(&up_axis, XMVector3Rotate(XMLoadFloat3(&up_axis),
		XMQuaternionRotationAxis(XMLoadFloat3(&target) - XMLoadFloat3(&position), angle)));
}

void TargetCamera::RotateHeading(float angle){
	headingAngle += angle;
}

XMFLOAT3 TargetCamera::at() {
	XMFLOAT3 result;
	XMVECTOR direction = XMLoadFloat3(&target) - XMLoadFloat3(&position);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&up_axis));
	XMVECTOR At = XMVector3Rotate(direction, XMQuaternionRotationAxis(right_axis, headingAngle));
	At = XMLoadFloat3(&position) + XMVector3Normalize(At);
	XMStoreFloat3(&result, At);
	return result;
}

XMFLOAT3 TargetCamera::up() {
	XMFLOAT3 result;
	XMVECTOR direction = XMLoadFloat3(&target) - XMLoadFloat3(&position);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&up_axis));
	XMStoreFloat3(&result, XMVector3Rotate(XMLoadFloat3(&up_axis),
		XMQuaternionRotationAxis(right_axis, headingAngle)));
	return result; 
}

float TargetCamera::targetDist(){
	float len;
	XMStoreFloat(&len, XMVector3Length(XMLoadFloat3(&target) - XMLoadFloat3(&position)));
	return len;
}

TargetCamera::~TargetCamera(){}
