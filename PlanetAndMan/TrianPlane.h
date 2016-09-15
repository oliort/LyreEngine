#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>
#include <map>
#include <utility>
#include <math.h>

#include "structs.h"
#include "TrianPoint.h"

#define DIR_UP		true
#define DIR_DOWN	false

#define INVALID_HALF -1

class TrianSphere;

class TrianPlane {
	friend class TrianSphere;

	TrianSphere* sphere;
	TrianPoint* points[3];
	bool direction;
	XMFLOAT3 normal;
	bool divided;
	TrianPoint* halfs[3];
	int validHalf;
	TrianPlane* father;
	TrianPlane* children[4];

	TrianPlane* neighbours[3];

	void clearChildren();

public:
	TrianPlane(TrianSphere* trianSphere, 
		TrianPoint* point1, TrianPoint* point2, TrianPoint* point3,
		bool dir, TrianPlane* fatherPlane = nullptr);
	void divide(int depth = 1);
	void setHalf(int halfInd, TrianPoint * half);
	int getTrianglesAmount();
	DWORD getTrianPointIndex(int pointInd);
	DWORD getHalfIndex(int halfInd);
	~TrianPlane();
};
