#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>

class PlanetAndMan;

class Character
{
	PlanetAndMan* pApplication;
	XMFLOAT3 currentPositionVec;
	XMFLOAT3 forwardVec;
	float currentHeight;
public:
	Character(PlanetAndMan* application);
	bool tryMoveForward(float angle);
	void rotateLeft(float angle);
	void rotateRight(float angle);
	XMFLOAT3 pos();
	XMFLOAT4X4 transformation();
	XMFLOAT3 at();
	XMFLOAT3 up();
	~Character();
};

