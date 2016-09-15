#include "pam_math.h"

float quinticSmooth(float x){
	return x*x*x*(x*(6.f*x - 15.f) + 10.f);
}

float quinticSmoothDeriv(float x){
	return 30 * x*x*(x - 1)*(x - 1);
}

float gridGrad(int hash, float x, float y, float z) {
	switch (hash & 0xF) {
	case 0xC:
	case 0x0: return  x + y;
	case 0xE:
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0xD:
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xF:
	case 0xB: return -y - z;
	default: return 0; //never happens
	}
}

float clamp(float x, float a, float b){
	return x < a ? a : (x > b ? b : x);
}