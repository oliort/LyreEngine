#pragma once

#include <windows.h>
#include <d3d11.h>
#include <xnamath.h>

#define EPS 0.00001f

float quinticSmooth(float x);

float quinticSmoothDeriv(float x);

float gridGrad(int hash, float x, float y, float z);

float clamp(float x, float a, float b);
