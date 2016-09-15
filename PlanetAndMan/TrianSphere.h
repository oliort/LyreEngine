#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>
#include <map>
#include <utility>
#include <math.h>
#include <forward_list>

#include "TrianPoint.h"
#include "TrianPlane.h"
#include "structs.h"

#define ICOS_PLANES 20
#define ICOS_POINTS 12

#define TWO_PI_DIV5         1.2566370614
#define ICOS_BELT_Y			0.4472135955
#define ICOS_BELT_RAD       0.8944271910

class TrianSphere {
	float rad;

	TrianPlane* icosahedron[ICOS_PLANES];
	std::forward_list<TrianPoint*> points;

	std::vector<SimpleVertex> vertices;
	std::vector<DWORD> indices;

	int pointsCount;
	int planesCount;

	void buildIcosahedron();
	void buildTrianVIndsR(TrianPlane* trian);
	void buildTrianCPIndsR(TrianPlane* trian);
	SimpleVertex* getVertexByIndex(DWORD index);

public:
	TrianSphere(float radius);
	void divide(unsigned depth);
	void rebuildVertexIndicesBuffer();
	void rebuildCPIndicesBuffer();
	//creates a point between two others
	TrianPoint* createHalf(TrianPoint* point1, TrianPoint* point2);
	SimpleVertex* getVertexByTrianPoint(TrianPoint *point);
	const std::vector<SimpleVertex>& getVertices();
	void fillAllTriangleVertices(std::vector<SimpleVertex>& allTriangleVertices);
	const std::vector<DWORD>& getIndices();
	void updatePlanesAmount();
	int getTrianPlanesAmount();
	int getTrianPointsAmount();
	int getIndicesAmount();
	~TrianSphere();
};
