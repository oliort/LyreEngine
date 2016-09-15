#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>
#include <vector>
#include <map>
#include <utility>
#include <math.h>

#include "structs.h"

#define ADD_NEIG_FIR	1
#define ADD_NEIG_SEC	2
#define ADD_NEIG_THI	4
#define ADD_NEIG_ALL	7

class TrianSphere;
class TrianPlane;

class TrianPoint {
	friend class TrianPlane;
	friend class TrianSphere;

	DWORD index;

public:
	TrianPoint(DWORD ind);
	DWORD getIndex();
};
