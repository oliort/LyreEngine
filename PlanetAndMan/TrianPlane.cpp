#include "TrianPlane.h"

#include "TrianPoint.h"
#include "TrianSphere.h"

TrianPlane::TrianPlane(TrianSphere* trianSphere, 
	TrianPoint* point1, TrianPoint* point2, TrianPoint* point3,
	bool dir, TrianPlane* fatherPlane)
	: sphere(trianSphere), direction(dir), father(fatherPlane),
	points{ point1, point2, point3 },
	halfs{ nullptr, nullptr, nullptr }, validHalf(INVALID_HALF),
	children{ nullptr, nullptr, nullptr }, divided(false) {}

void TrianPlane::divide(int depth) {
	if (depth < 1) return;

	if (!divided) {
		//divide father's neighbour if it is not divided
		try {
			if (neighbours[0] == nullptr) father->neighbours[0]->divide();
			if (neighbours[1] == nullptr) father->neighbours[1]->divide();
			if (neighbours[2] == nullptr) father->neighbours[2]->divide();
		}
		catch (...) { throw std::exception("TrianSphere Fail Structure Exception"); }

		//creating halfs if they don't exist
		///halfs are indexed correctrly for inner (children[3]) opposite-directional triangle
		if (halfs[0] == nullptr) {
			halfs[0] = sphere->createHalf(points[1], points[2]);
			///setting neighbour's halfs even if they aren't divided
			///neighbour with index "0" must be opposite-directional
			neighbours[0]->setHalf(0, halfs[0]);
		}
		if (halfs[1] == nullptr) {
			halfs[1] = sphere->createHalf(points[2], points[0]);
			if (neighbours[1]->direction == direction)
				neighbours[1]->setHalf(2, halfs[1]);
			else neighbours[1]->setHalf(1, halfs[1]);
		}
		if (halfs[2] == nullptr) {
			halfs[2] = sphere->createHalf(points[0], points[1]);
			if (neighbours[2]->direction == direction)
				neighbours[2]->setHalf(1, halfs[2]);
			else neighbours[2]->setHalf(2, halfs[2]);
		}

		//creating children
		children[0] = new TrianPlane(sphere, points[0], halfs[2], halfs[1], direction, this);
		children[1] = new TrianPlane(sphere, halfs[2], points[1], halfs[0], direction, this);
		children[2] = new TrianPlane(sphere, halfs[1], halfs[0], points[2], direction, this);
		///opposite-directional triangle
		children[3] = new TrianPlane(sphere, halfs[0], halfs[1], halfs[2], direction ^ true, this);

		//setting children's neighbours
		///other children for inner child
		children[3]->neighbours[0] = children[0];
		children[3]->neighbours[1] = children[1];
		children[3]->neighbours[2] = children[2];
		///inner child for other children
		children[0]->neighbours[0] = children[3];
		children[1]->neighbours[1] = children[3];
		children[2]->neighbours[2] = children[3];
		///outer neighbours
		if (neighbours[0]->divided) {
			children[1]->neighbours[0] = neighbours[0]->children[2];
			children[2]->neighbours[0] = neighbours[0]->children[1];
			neighbours[0]->children[1]->neighbours[0] = children[2];
			neighbours[0]->children[2]->neighbours[0] = children[1];
		}
		if (neighbours[1]->divided) {
			if (neighbours[1]->direction != direction) {
				children[0]->neighbours[1] = neighbours[1]->children[2];
				children[2]->neighbours[1] = neighbours[1]->children[0];
				neighbours[1]->children[0]->neighbours[1] = children[2];
				neighbours[1]->children[2]->neighbours[1] = children[0];
			}
			else {
				children[0]->neighbours[1] = neighbours[1]->children[0];
				children[2]->neighbours[1] = neighbours[1]->children[1];
				neighbours[1]->children[0]->neighbours[2] = children[0];
				neighbours[1]->children[1]->neighbours[2] = children[2];
			}
		}
		if (neighbours[2]->divided) {
			if (neighbours[2]->direction != direction) {
				children[0]->neighbours[2] = neighbours[2]->children[1];
				children[1]->neighbours[2] = neighbours[2]->children[0];
				neighbours[2]->children[0]->neighbours[2] = children[1];
				neighbours[2]->children[1]->neighbours[2] = children[0];
			}
			else {
				children[0]->neighbours[2] = neighbours[2]->children[0];
				children[1]->neighbours[2] = neighbours[2]->children[2];
				neighbours[2]->children[0]->neighbours[1] = children[0];
				neighbours[2]->children[2]->neighbours[1] = children[1];
			}
		}

		divided = true;
	}

	for (int i = 0; i < 4; ++i) {
		children[i]->divide(depth-1);
	}
}

void TrianPlane::setHalf(int halfInd, TrianPoint * half) {
	if (divided) throw std::exception("No permission to change divided TrianPlane!");
	halfs[halfInd] = half;
	if (validHalf != INVALID_HALF) divide();
	else validHalf = halfInd;
}

DWORD TrianPlane::getTrianPointIndex(int pointInd) {
	return points[pointInd]->index;
}

DWORD TrianPlane::getHalfIndex(int halfInd){
	return halfs[halfInd]->index;
}
int TrianPlane::getTrianglesAmount() {
	if (!divided) {
		if (validHalf != INVALID_HALF) return 2;
		else return 1;
	}
	return children[0]->getTrianglesAmount() +
		children[1]->getTrianglesAmount() +
		children[2]->getTrianglesAmount() +
		children[3]->getTrianglesAmount();
}

void TrianPlane::clearChildren() {
	for (int i = 0; i < 4; ++i) delete children[i];
}

TrianPlane::~TrianPlane() {
	if (divided) clearChildren();
}