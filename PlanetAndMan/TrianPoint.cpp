#include "TrianPoint.h"

#include "TrianPlane.h"
#include "TrianSphere.h"

TrianPoint::TrianPoint(DWORD ind)
	: index(ind){}


DWORD TrianPoint::getIndex(){
	return index;
}
