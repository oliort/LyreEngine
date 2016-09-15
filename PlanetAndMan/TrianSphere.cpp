#include "TrianSphere.h"

#include "TrianPlane.h"
#include "TrianPoint.h"

#include <exception>
#include <map>
#include <functional>

TrianSphere::TrianSphere(float radius)
	: rad(radius),
	vertices(0), indices(0), points(0),
	pointsCount(0), planesCount(0) {
	buildIcosahedron();
}

void TrianSphere::buildIcosahedron() {
	//creating vertices
	std::vector<XMFLOAT3> icasahedronVertices(ICOS_POINTS);
	///poles
	icasahedronVertices[0] = XMFLOAT3(0, rad, 0);
	icasahedronVertices[11] = XMFLOAT3(0, -rad, 0);
	///upper belt
	float angle = 0;
	for (int i = 0; i < 5; ++i) {
		icasahedronVertices[1 + i] = XMFLOAT3(
			rad*ICOS_BELT_RAD*sin(angle),
			rad*ICOS_BELT_Y,
			rad*ICOS_BELT_RAD*cos(angle));
		angle += TWO_PI_DIV5;
	}
	///lower belt
	angle = TWO_PI_DIV5 / 2;
	for (int i = 0; i < 5; ++i) {
		icasahedronVertices[6 + i] = XMFLOAT3(
			rad*ICOS_BELT_RAD*sin(angle),
			-rad*ICOS_BELT_Y,
			rad*ICOS_BELT_RAD*cos(angle));
		angle += TWO_PI_DIV5;
	}
	
	std::map<int, TrianPoint*> alreadyEvolved;
	std::function<TrianPoint*(int)> evolveIcosVertex = 
		[&alreadyEvolved, icasahedronVertices, this](int ind){
		auto it = alreadyEvolved.find(ind);
		if (it != alreadyEvolved.end())
			return it->second;
		DWORD newInd = pointsCount++;
		vertices.push_back(SimpleVertex());
		vertices[newInd].pos = icasahedronVertices[ind];
		TrianPoint* newPoint = new TrianPoint(newInd);
		points.push_front(newPoint);
		alreadyEvolved[ind] = newPoint;
		return newPoint;
	};

	//creating TrianPlanes
	///upper dome
	for (int i = 0; i < 5; ++i)
		icosahedron[i] = new TrianPlane(this,
			evolveIcosVertex(0),
			evolveIcosVertex(1 + (i + 1) % 5),
			evolveIcosVertex(1 + i), 
			DIR_UP);
	///upper belt
	for (int i = 0; i < 5; ++i)
		icosahedron[5 + i] = new TrianPlane(this,
			evolveIcosVertex(6 + i),
			evolveIcosVertex(1 + i),
			evolveIcosVertex(1 + (i + 1) % 5),
			DIR_DOWN);
	///lower belt
	for (int i = 0; i < 5; ++i)
		icosahedron[10 + i] = new TrianPlane(this,
			evolveIcosVertex(1 + (i + 1) % 5),
			evolveIcosVertex(6 + (i + 1) % 5),
			evolveIcosVertex(6 + i),
			DIR_UP);
	///lower dome
	for (int i = 0; i < 5; ++i)
		icosahedron[15 + i] = new TrianPlane(this,
			evolveIcosVertex(11),
			evolveIcosVertex(6 + i),
			evolveIcosVertex(6 + (i + 1) % 5),
			DIR_DOWN);

	//setting neighbours
	///upper dome
	for (int i = 0; i < 5; ++i){
		icosahedron[i]->neighbours[0] = icosahedron[5 + i];
		icosahedron[i]->neighbours[1] = icosahedron[(i + 4) % 5];
		icosahedron[i]->neighbours[2] = icosahedron[(i + 1) % 5];}
	///upper belt
	for (int i = 0; i < 5; ++i){
		icosahedron[5 + i]->neighbours[0] = icosahedron[i];
		icosahedron[5 + i]->neighbours[1] = icosahedron[10 + i];
		icosahedron[5 + i]->neighbours[2] = icosahedron[10 + (i + 4) % 5];}
	///lower belt
	for (int i = 0; i < 5; ++i){
		icosahedron[10 + i]->neighbours[0] = icosahedron[15 + i];
		icosahedron[10 + i]->neighbours[1] = icosahedron[5 + i];
		icosahedron[10 + i]->neighbours[2] = icosahedron[5 + (i + 1) % 5];
	}
	///lower dome
	for (int i = 0; i < 5; ++i){
		icosahedron[15 + i]->neighbours[0] = icosahedron[10 + i];
		icosahedron[15 + i]->neighbours[1] = icosahedron[15 + (i + 1) % 5];
		icosahedron[15 + i]->neighbours[2] = icosahedron[15 + (i + 4) % 5];}

	planesCount = ICOS_PLANES;
}

void TrianSphere::divide(unsigned depth) {
	for (int i = 0; i < ICOS_PLANES; ++i) {
		icosahedron[i]->divide(depth);
	}
}

void TrianSphere::rebuildVertexIndicesBuffer() {
	indices.clear();
	indices.reserve(planesCount * 3);
	for (int i = 0; i < ICOS_PLANES; ++i) {
		buildTrianVIndsR(icosahedron[i]);
	}
}

void TrianSphere::rebuildCPIndicesBuffer(){
	indices.clear();
	indices.reserve(planesCount * 6);
	for (int i = 0; i < ICOS_PLANES; ++i) {
		buildTrianCPIndsR(icosahedron[i]);
	}
}

TrianPoint* TrianSphere::createHalf(TrianPoint* point1, TrianPoint* point2) {
	DWORD newInd = pointsCount++;
	vertices.push_back(SimpleVertex());
	XMStoreFloat3(&(vertices[newInd].pos), XMVector3Normalize({
		(getVertexByTrianPoint(point1)->pos.x + getVertexByTrianPoint(point2)->pos.x) / 2,
		(getVertexByTrianPoint(point1)->pos.y + getVertexByTrianPoint(point2)->pos.y) / 2,
		(getVertexByTrianPoint(point1)->pos.z + getVertexByTrianPoint(point2)->pos.z) / 2,
		0 }) * rad);
	TrianPoint* newPoint = new TrianPoint(newInd);
	points.push_front(newPoint);
	return newPoint;
}

const std::vector<SimpleVertex>& TrianSphere::getVertices() {
	return vertices;
}

void TrianSphere::fillAllTriangleVertices(std::vector<SimpleVertex> &allTriangleVertices){
	allTriangleVertices.clear();
	for (int i = 0; i < indices.size(); i++) {
		allTriangleVertices.push_back(vertices[indices[i]]);
	}
}

const std::vector<DWORD>& TrianSphere::getIndices() {
	return indices;
}

void TrianSphere::updatePlanesAmount() {
	planesCount = 0;
	for (int i = 0; i < ICOS_PLANES; ++i) {
		planesCount += icosahedron[i]->getTrianglesAmount();
	}
}

SimpleVertex * TrianSphere::getVertexByIndex(DWORD index){
	return &(vertices[index]);
}

SimpleVertex* TrianSphere::getVertexByTrianPoint(TrianPoint * point){
	return &(vertices[point->getIndex()]);
}

int TrianSphere::getTrianPlanesAmount() {
	return planesCount;
}

int TrianSphere::getTrianPointsAmount() {
	return pointsCount;
}

int TrianSphere::getIndicesAmount() {
	return indices.size();
}

TrianSphere::~TrianSphere() {
	for (int i = 0; i < ICOS_PLANES; ++i) delete icosahedron[i];
	for (auto it = points.begin(); it != points.end(); ++it) delete (*it);
}

void TrianSphere::buildTrianVIndsR(TrianPlane* trian) {
	if (trian->divided) {
		buildTrianVIndsR(trian->children[0]);
		buildTrianVIndsR(trian->children[1]);
		buildTrianVIndsR(trian->children[2]);
		buildTrianVIndsR(trian->children[3]);
	}
	//separate in two triangles to cover neighbour's half (if it exists)
	else switch (trian->validHalf) {
	case 0:
		indices.push_back(trian->getHalfIndex(0));
		indices.push_back(trian->getTrianPointIndex(1));
		indices.push_back(trian->getTrianPointIndex(0));
		////
		indices.push_back(trian->getHalfIndex(0));
		indices.push_back(trian->getTrianPointIndex(0));
		indices.push_back(trian->getTrianPointIndex(2));
		break;
	case 1:
		indices.push_back(trian->getHalfIndex(1));
		indices.push_back(trian->getTrianPointIndex(1));
		indices.push_back(trian->getTrianPointIndex(0));
		////
		indices.push_back(trian->getHalfIndex(1));
		indices.push_back(trian->getTrianPointIndex(2));
		indices.push_back(trian->getTrianPointIndex(1));
		break;
	case 2:
		indices.push_back(trian->getHalfIndex(2));
		indices.push_back(trian->getTrianPointIndex(0));
		indices.push_back(trian->getTrianPointIndex(2));
		////
		indices.push_back(trian->getHalfIndex(2));
		indices.push_back(trian->getTrianPointIndex(2));
		indices.push_back(trian->getTrianPointIndex(1));
		break;
	default://no halfs exist
		indices.push_back(trian->getTrianPointIndex(2));
		indices.push_back(trian->getTrianPointIndex(1));
		indices.push_back(trian->getTrianPointIndex(0));
		break;
	}
}

void TrianSphere::buildTrianCPIndsR(TrianPlane * trian){
	if (trian->divided) {
		buildTrianCPIndsR(trian->children[0]);
		buildTrianCPIndsR(trian->children[1]);
		buildTrianCPIndsR(trian->children[2]);
		buildTrianCPIndsR(trian->children[3]);
	}
	//no half separation for control point patches
	else {
		indices.push_back(trian->getTrianPointIndex(2));
		indices.push_back(trian->getTrianPointIndex(1));
		indices.push_back(trian->getTrianPointIndex(0));
		////
		if (trian->neighbours[2]->direction != trian->direction)
			indices.push_back(trian->neighbours[2]->getTrianPointIndex(2));
		else indices.push_back(trian->neighbours[2]->getTrianPointIndex(1));
		if (trian->neighbours[1]->direction != trian->direction)
			indices.push_back(trian->neighbours[1]->getTrianPointIndex(1));
		else indices.push_back(trian->neighbours[1]->getTrianPointIndex(2));
		indices.push_back(trian->neighbours[0]->getTrianPointIndex(0));
	}
}
