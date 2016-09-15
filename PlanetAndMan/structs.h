#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>
#include <xnamath.h>
#include <vector>

struct SimpleVertex
{
	XMFLOAT3 pos;
};

struct ColoredVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

struct TerrainHSConstantBuffer
{
	XMFLOAT4 CameraPosAndVisibleDist;
	float MinDist;
	float MaxDist;
	float MinLOD;
	float MaxLOD;
};

struct UINT3 {
	unsigned int x, y, z;
};

struct UINT4 {
	unsigned int v1, v2, v3, v4;
};

struct TerrainDSPerlinConstantBuffer 
{
	XMFLOAT4 PContinents; //water or ground?
	XMFLOAT4 PVallMount; //ground : valleys or mountains?		-no deriv.
	XMFLOAT4 PPlainHills; //valleys : plains or hills			-no deriv.
	XMFLOAT4 PErosRidges; //mountains : erosion or ridges		-no deriv.
	XMFLOAT4 PHills;
	XMFLOAT4 PErosion;
	XMFLOAT4 PRidges;
	XMFLOAT2 PVallMountThreshold;
	XMFLOAT2 PPlainHillsThreshold;
	XMFLOAT2 PErosRidgesThreshold;
	XMFLOAT2 PMountHeightThreshold;
};

struct ViewProjLightConstantBuffer
{
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProjection;
	XMFLOAT4 Light;
};

struct PlanetConstantBuffer
{
	XMFLOAT4 PlanetPos;
};

struct CharacterVSConstantBuffer
{
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProjection;
	XMFLOAT4X4 mManRotate;
	XMFLOAT4 vManTranslate;
};

struct TextureLayer {
	XMFLOAT3 height; //min, max, lowerEps, upperEps
};

#endif
