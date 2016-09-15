#include "Character.h"

#include "PlanetAndMan.h"

Character::Character(PlanetAndMan * application)
	: pApplication(application){
	XMStoreFloat3(&currentPositionVec, XMVector3Normalize(XMVECTOR{
		rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX }));
	currentHeight = pApplication->getPerlinHeight(currentPositionVec);
	while (!(currentHeight > 0)) {
		XMStoreFloat3(&currentPositionVec, XMVector3Normalize(XMVECTOR{
			rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX }));
		currentHeight = pApplication->getPerlinHeight(currentPositionVec);
	}
	XMStoreFloat3(&forwardVec, XMVector3Normalize(XMVector3Cross({
		sqrt(PI), PI, PI*PI }, XMLoadFloat3(&currentPositionVec))));
}

bool Character::tryMoveForward(float angle){
	XMVECTOR right_axis = XMVector3Cross(XMLoadFloat3(&currentPositionVec), XMLoadFloat3(&forwardVec));
	XMFLOAT3 newPositionVec;
	float newHeight;
	XMStoreFloat3(&newPositionVec, XMVector3Normalize(XMVector3Rotate(XMLoadFloat3(&currentPositionVec), XMQuaternionRotationAxis(right_axis, angle))));
	newHeight = pApplication->getPerlinHeight(newPositionVec);
	if ((newHeight < 0) || (newHeight - currentHeight > MAX_HEIGHT_DIFFERENCE)) return false;
	XMStoreFloat3(&forwardVec, XMVector3Normalize(XMVector3Cross(right_axis, XMLoadFloat3(&newPositionVec))));
	currentPositionVec = newPositionVec;
	currentHeight = newHeight;
	return true;
}

void Character::rotateLeft(float angle){
	XMStoreFloat3(&forwardVec, XMVector3Rotate(XMLoadFloat3(&forwardVec),
		XMQuaternionRotationAxis(-XMLoadFloat3(&currentPositionVec), angle)));
}

void Character::rotateRight(float angle){
	XMStoreFloat3(&forwardVec, XMVector3Rotate(XMLoadFloat3(&forwardVec),
		XMQuaternionRotationAxis(-XMLoadFloat3(&currentPositionVec), -angle)));
}

XMFLOAT3 Character::pos(){
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, XMLoadFloat3(&currentPositionVec) * (PLANET_RAD + MAN_HEIGHT + currentHeight));
	return pos;
}

XMFLOAT4X4 Character::transformation(){
	XMFLOAT3 right;
	XMStoreFloat3(&right, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&currentPositionVec), XMLoadFloat3(&forwardVec))));
	return XMFLOAT4X4{
		right.x,				right.y,				right.z,				0.f,
		currentPositionVec.x,	currentPositionVec.y,	currentPositionVec.z,	0.f,
		forwardVec.x,			forwardVec.y,			forwardVec.z,			0.f,
		0.f,					0.f,					0.f,					1.f
	};
}

XMFLOAT3 Character::at() {
	XMFLOAT3 at;
	XMStoreFloat3(&at, XMLoadFloat3(&pos()) + XMLoadFloat3(&forwardVec));
	return at;
}

XMFLOAT3 Character::up() {
	return currentPositionVec;
}

Character::~Character()
{
}
