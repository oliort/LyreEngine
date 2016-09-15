#include "Camera.h"

Camera::Camera(float x, float y, float z)
	: position  { x,y,z }, 
	up_axis{ 0.f,1.f,0.f }, 
	field_of_view(90.f) { }

void Camera::GoTo(float x, float y, float z){
	XMStoreFloat3(&position, XMVectorSet(x, y, z, 0));
}

void Camera::Move(float x, float y, float z){
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMVectorSet(x, y, z, 0));
}

void Camera::setFOV(float value) {
	field_of_view = value;
}

XMFLOAT3 Camera::pos() { return position; }
XMFLOAT3 Camera::up() { return up_axis; }
float Camera::FOV() { return field_of_view; }

Camera::~Camera(){}