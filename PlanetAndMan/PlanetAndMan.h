#pragma once

#include <d3dcompiler.h>
#include <d3dx11.h>
#include <atlbase.h>

#include "structs.h"
#include "Camera.h"
#include "ControlsApplier.h"
#include "TrianSphere.h"
#include "Character.h"

#define WND_WIDTH   1900
#define WND_HEIGHT  1000

#define PLANET_RAD 10.f
#define MAN_HEIGHT 0.01f
#define MAX_HEIGHT_DIFFERENCE 0.001f
#define ADDITIONAL_HORIZON_RAD 0.5f

#define INITIAL_DIVISION_DEPTH 5

class PlanetAndMan final {
private:
	HINSTANCE					hInstance = nullptr;
	HWND						hWindow = nullptr;

	D3D_DRIVER_TYPE				driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			featureLevel = D3D_FEATURE_LEVEL_11_0;

	CComPtr<ID3D11Device>				iDevice = nullptr;
	CComPtr<ID3D11DeviceContext>		iImmediateContext = nullptr;

	CComPtr<IDXGISwapChain>				iSwapChain = nullptr;

	CComPtr<ID3D11RenderTargetView>		iRTV = nullptr;

	CComPtr<ID3D11DepthStencilState>	iDSState = nullptr;
	CComPtr<ID3D11DepthStencilView>		iDSV = nullptr;

	CComPtr<ID3D11BlendState>			iBlendState = nullptr;
	
	//						TERRAIN
	CComPtr<ID3D11VertexShader>			iTerrainVS = nullptr;
	CComPtr<ID3D11HullShader>			iTerrainHS = nullptr;
	CComPtr<ID3D11DomainShader>			iTerrainDS = nullptr;
	CComPtr<ID3D11PixelShader>			iTerrainPS = nullptr;

	CComPtr<ID3D11InputLayout>			iTerrainVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				iTerrainVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				iTerrainIndexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				iTerrainHSConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				iTerrainDSPerlinConstantBuffer = nullptr;

	CComPtr<ID3D11ShaderResourceView>	iTerrainTexturesSRV = nullptr;
	CComPtr<ID3D11ShaderResourceView>	iTerrainTextureLayersSRV = nullptr;
	////////////////////////////////////////////////////////////

	//						WATER
	CComPtr<ID3D11VertexShader>			iWaterVS = nullptr;
	CComPtr<ID3D11PixelShader>			iWaterPS = nullptr;

	CComPtr<ID3D11InputLayout>			iWaterVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				iWaterVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				iWaterIndexBuffer = nullptr;

	CComPtr<ID3D11ShaderResourceView>	iWaterTextureSRV = nullptr;
	////////////////////////////////////////////////////////////

	//						MAN
	CComPtr<ID3D11VertexShader>			iCharacterVS = nullptr;
	CComPtr<ID3D11PixelShader>			iCharacterPS = nullptr;

	CComPtr<ID3D11InputLayout>			iCharacterVertexLayout = nullptr;
	CComPtr<ID3D11Buffer>				iCharacterVertexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				iCharacterIndexBuffer = nullptr;

	CComPtr<ID3D11Buffer>				iCharacterVSConstantBuffer = nullptr;
	////////////////////////////////////////////////////////////

	//						SHARED
	CComPtr<ID3D11Buffer>				iViewProjLightConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				iPlanetConstantBuffer = nullptr;

	CComPtr<ID3D11SamplerState>			iTex2DSampler;

	CComPtr<ID3D11ShaderResourceView>	iPerlinPermutSRV = nullptr; //2(256*2=512) same hashing 0-255 permutations

	CComPtr<ID3D11ShaderResourceView>	iQuinticSmoothLTSRV = nullptr; //0-255 for smooth, 256-511 for it's derivative
	////////////////////////////////////////////////////////////

	CComPtr<ID3D11RasterizerState>		iRasterizerStateWireframe = nullptr;
	CComPtr<ID3D11RasterizerState>		iRasterizerStateSolid = nullptr;

	XMMATRIX					view;
	XMMATRIX					projection;

	TrianSphere*				terrain;
	TrianSphere*				water;
	Character*					man;

	CComPtr<ID3D11SamplerState>			iLTSampler; //Lookup tables' sampler

	std::vector<int> perlinPermutations;
	TerrainDSPerlinConstantBuffer perlinParams;

	bool bCameraPlanetOrMan;
	bool bRenderPlanetOrMan;
	bool bSolidOrWireframe;
	Camera* pCamera;
	ControlsApplier* pControlsApplier;

	HRESULT InitWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	HRESULT InitDevice();

	HRESULT InitSharedResources();

	HRESULT InitTerrainGeometry();
	HRESULT GenerateTerrain();

	HRESULT InitWaterGeometry();
	HRESULT GenerateWater();

	HRESULT InitManGeometry();
	HRESULT GenerateMan();

	HRESULT ReadShaderFromFile(WCHAR * szFileName, std::vector<char>& shaderBytecode);

	HRESULT PlanetAndMan::CreateTexture2DArraySRV(ID3D11Device* device, ID3D11DeviceContext* context,
		const std::vector<WCHAR*>& filenames, D3DX11_IMAGE_LOAD_INFO* textureInfo, ID3D11ShaderResourceView** srv);

	void GetClientWH(UINT&, UINT&);

public:
	PlanetAndMan(HINSTANCE hInstance, int nCmdShow, WNDPROC WndProc);//to function
	void Render();
	Camera* getCamera();
	Character* getMan();
	void switchCamera();
	void switchRender();
	void switchRasterizer();
	float perlinNoise(XMFLOAT3 pos);
	float getPerlinHeight(XMFLOAT3 normal);
	ControlsApplier* getControlsApplier();
	~PlanetAndMan();//to function
};