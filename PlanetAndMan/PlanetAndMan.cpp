#include "PlanetAndMan.h"

#include <d3d11.h>
#include <windows.h>
#include <d3dcompiler.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <exception>
#include <fstream>

#include "Resource.h"

#include "KeyTranslatorDefault.h"
#include "TargetCamera.h"

#include "pam_math.h"
#include "pam_macro.h"

PlanetAndMan::PlanetAndMan(HINSTANCE hInstance, int nCmdShow, WNDPROC WndProc)
	: pCamera(new TargetCamera(0, 0, PLANET_RAD*2, 0, 0, 0)), pControlsApplier(new ControlsApplier(this)),
	bCameraPlanetOrMan(true), bRenderPlanetOrMan(true), bSolidOrWireframe(true){
	if (!SUCCEEDED(InitWindow(hInstance, nCmdShow, WndProc))) throw std::exception("Failed to initialize window!");
	if (!SUCCEEDED(InitDevice())) throw std::exception("Failed to initialize D3D11 device!");
	if (!SUCCEEDED(InitSharedResources())) throw std::exception("Failed to initialize D3D11 device!");
	if (!SUCCEEDED(InitTerrainGeometry())) throw std::exception("Failed to initialize terrain!");
	if (!SUCCEEDED(InitWaterGeometry())) throw std::exception("Failed to initialize water!");
	if (!SUCCEEDED(InitManGeometry())) throw std::exception("Failed to initialize hero!");
}

void PlanetAndMan::Render() {
	static DWORD previousTime = GetTickCount();
	static DWORD tpf = 0.0f;//milliseconds
	tpf = (GetTickCount() - previousTime);
	previousTime = GetTickCount();
	
	static XMVECTOR rotAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	static XMFLOAT4 Light(0.0f, 0.0f, 1.0f, 0.0f);
	_XMMATRIX mRotate = XMMatrixRotationAxis(rotAxis, -0.00001f * tpf);
	XMStoreFloat4(&Light, XMVector3Transform(XMLoadFloat4(&Light), mRotate));
	Light.w = 0.01f;

	static const float additionalHorizonDist = sqrt(ADDITIONAL_HORIZON_RAD*(ADDITIONAL_HORIZON_RAD + 2.f * PLANET_RAD));
	XMFLOAT3 camPos;
	float height;
	float visiblePlanetDist;
	if (bRenderPlanetOrMan) {
		camPos = pCamera->pos();
		height = sqrt(camPos.x*camPos.x + camPos.y*camPos.y + camPos.z*camPos.z) - PLANET_RAD;
		visiblePlanetDist = sqrt(height*(height + 2.f * PLANET_RAD)) + additionalHorizonDist;
	}
	else {
		camPos = man->pos();
		height = sqrt(camPos.x*camPos.x + camPos.y*camPos.y + camPos.z*camPos.z) - PLANET_RAD;
		visiblePlanetDist = sqrt(height*(height + 2.f * PLANET_RAD)) + additionalHorizonDist;
	}

	XMVECTOR vEye;
	XMVECTOR vAt;
	XMVECTOR vUp;
	if (bCameraPlanetOrMan) {
		vEye = XMLoadFloat3(&pCamera->pos());
		vAt = XMLoadFloat3(&pCamera->at());
		vUp = XMLoadFloat3(&pCamera->up());
	}
	else {
		vEye = XMLoadFloat3(&man->pos());
		vAt = XMLoadFloat3(&man->at());
		vUp = XMLoadFloat3(&man->up());
	}
	
	view = XMMatrixLookAtLH(vEye, vAt, vUp);
	projection = XMMatrixPerspectiveFovLH(3 * XM_PIDIV2 / 4, WND_WIDTH / (FLOAT)WND_HEIGHT, visiblePlanetDist/1000.f, visiblePlanetDist);

	float sky;
	XMStoreFloat(&sky, XMVector3Dot(XMVector3Normalize(vEye), XMLoadFloat4(&Light)));
	float ClearColor[4] = { 0.3f*sky, 0.5f*sky, 0.9f*sky, 1.0f };
	iImmediateContext->ClearRenderTargetView(iRTV, ClearColor);
	iImmediateContext->ClearDepthStencilView(iDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	if (bSolidOrWireframe) {
		iImmediateContext->RSSetState(iRasterizerStateSolid);
	}
	else {
		iImmediateContext->RSSetState(iRasterizerStateWireframe);
	}

	ViewProjLightConstantBuffer vplcb;
	XMStoreFloat4x4(&vplcb.mView, XMMatrixTranspose(view));
	XMStoreFloat4x4(&vplcb.mProjection, XMMatrixTranspose(projection));
	vplcb.Light = Light;
	iImmediateContext->UpdateSubresource(iViewProjLightConstantBuffer, 0, nullptr, &vplcb, 0, 0);

	PlanetConstantBuffer pcb;
	pcb.PlanetPos = XMFLOAT4(0.f, 0.f, 0.f, PLANET_RAD);	
	iImmediateContext->UpdateSubresource(iPlanetConstantBuffer, 0, nullptr, &pcb, 0, 0);

	{
		TerrainHSConstantBuffer hscb;
		hscb.CameraPosAndVisibleDist = XMFLOAT4(camPos.x, camPos.y, camPos.z, visiblePlanetDist);
		hscb.MinDist = 1.0f;
		hscb.MaxDist = visiblePlanetDist;
		hscb.MinLOD = 1.f;
		hscb.MaxLOD = 48.f;

		iImmediateContext->VSSetShader(iTerrainVS, nullptr, 0);
		iImmediateContext->HSSetShader(iTerrainHS, nullptr, 0);
		iImmediateContext->DSSetShader(iTerrainDS, nullptr, 0);
		iImmediateContext->PSSetShader(iTerrainPS, nullptr, 0);

		iImmediateContext->IASetInputLayout(iTerrainVertexLayout);
		UINT stride = sizeof(SimpleVertex); UINT offset = 0;
		iImmediateContext->IASetVertexBuffers(0, 1, &iTerrainVertexBuffer.p, &stride, &offset);
		iImmediateContext->IASetIndexBuffer(iTerrainIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		iImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST);

		iImmediateContext->UpdateSubresource(iTerrainHSConstantBuffer, 0, nullptr, &hscb, 0, 0);
		iImmediateContext->HSSetConstantBuffers(0, 1, &iTerrainHSConstantBuffer.p);

		iImmediateContext->UpdateSubresource(iTerrainDSPerlinConstantBuffer, 0, nullptr, &perlinParams, 0, 0);
		iImmediateContext->DSSetConstantBuffers(0, 1, &iViewProjLightConstantBuffer.p);
		iImmediateContext->DSSetConstantBuffers(1, 1, &iPlanetConstantBuffer.p);
		iImmediateContext->DSSetConstantBuffers(2, 1, &iTerrainDSPerlinConstantBuffer.p);
		iImmediateContext->DSSetShaderResources(0, 1, &iPerlinPermutSRV.p);
		iImmediateContext->DSSetShaderResources(1, 1, &iQuinticSmoothLTSRV.p);
		iImmediateContext->DSSetSamplers(0, 1, &iLTSampler.p);

		iImmediateContext->PSSetShaderResources(0, 1, &iTerrainTexturesSRV.p);
		iImmediateContext->PSSetShaderResources(1, 1, &iTerrainTextureLayersSRV.p);
		iImmediateContext->PSSetSamplers(0, 1, &iTex2DSampler.p);

		iImmediateContext->DrawIndexed(terrain->getTrianPlanesAmount() * 6, 0, 0);
	}

	{
		iImmediateContext->IASetInputLayout(iWaterVertexLayout);
		UINT stride = sizeof(SimpleVertex); UINT offset = 0;
		iImmediateContext->IASetVertexBuffers(0, 1, &iWaterVertexBuffer.p, &stride, &offset);
		iImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		iImmediateContext->VSSetShader(iWaterVS, nullptr, 0);
		iImmediateContext->HSSetShader(nullptr, nullptr, 0);
		iImmediateContext->DSSetShader(nullptr, nullptr, 0);
		iImmediateContext->PSSetShader(iWaterPS, nullptr, 0);

		iImmediateContext->VSSetConstantBuffers(0, 1, &iViewProjLightConstantBuffer.p);
		//////////
		static float waves = 0.f;
		pcb.PlanetPos = XMFLOAT4(0.f, 0.f, 0.f, waves+=0.01);
		iImmediateContext->UpdateSubresource(iPlanetConstantBuffer, 0, nullptr, &pcb, 0, 0);
		waves = waves - (int)waves;
		//////////
		iImmediateContext->VSSetConstantBuffers(1, 1, &iPlanetConstantBuffer.p);

		iImmediateContext->PSSetShaderResources(0, 1, &iWaterTextureSRV.p);
		iImmediateContext->PSSetSamplers(0, 1, &iTex2DSampler.p);

		iImmediateContext->Draw(water->getTrianPlanesAmount() * 3, 0);
	}

	if (bCameraPlanetOrMan) {
		CharacterVSConstantBuffer vscb;
		XMStoreFloat4x4(&vscb.mView, XMMatrixTranspose(view));
		XMStoreFloat4x4(&vscb.mProjection, XMMatrixTranspose(projection));
		vscb.mManRotate = man->transformation();
		vscb.vManTranslate = { man->pos().x, man->pos().y, man->pos().z, 0.f };

		iImmediateContext->IASetInputLayout(iCharacterVertexLayout);
		UINT stride = sizeof(ColoredVertex); UINT offset = 0;
		iImmediateContext->IASetVertexBuffers(0, 1, &iCharacterVertexBuffer.p, &stride, &offset);
		iImmediateContext->IASetIndexBuffer(iCharacterIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		iImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		iImmediateContext->VSSetShader(iCharacterVS, nullptr, 0);
		iImmediateContext->HSSetShader(nullptr, nullptr, 0);
		iImmediateContext->DSSetShader(nullptr, nullptr, 0);
		iImmediateContext->PSSetShader(iCharacterPS, nullptr, 0);

		iImmediateContext->UpdateSubresource(iCharacterVSConstantBuffer, 0, nullptr, &vscb, 0, 0);
		iImmediateContext->VSSetConstantBuffers(0, 1, &iCharacterVSConstantBuffer.p);

		iImmediateContext->DrawIndexed(12, 0, 0);
	}
	
	iSwapChain->Present(0, 0);
}

HRESULT PlanetAndMan::InitWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc) {
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_TUTORIAL1);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"PlanetAndMan";
		wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	}
	if (!RegisterClassEx(&wcex)) return E_FAIL;
	hInstance = hInst;
	RECT rc = { 0, 0, WND_WIDTH, WND_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	hWindow = CreateWindow(L"PlanetAndMan", L"Planet and Man", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
	if (!hWindow) return E_FAIL;
	ShowWindow(hWindow, nCmdShow);

	return S_OK;
}

HRESULT PlanetAndMan::InitDevice(){
	HRESULT hr;

	UINT width, height;
	GetClientWH(width, height);

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	{
		ZeroStruct(swapChainDesc);
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWindow;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
	}
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, 0, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &swapChainDesc, &iSwapChain, &iDevice, &featureLevel, &iImmediateContext);
		if (SUCCEEDED(hr)) break;
	}
	if (FAILED(hr)) return hr;

	//Creating RTV
	CComPtr<ID3D11Texture2D> pSwapChainBuffer = nullptr;
	hr = iSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pSwapChainBuffer);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateRenderTargetView(pSwapChainBuffer, nullptr, &iRTV);
	if (FAILED(hr)) return hr;

	//Creating DSBuffer
	CComPtr<ID3D11Texture2D> pDSBuffer = nullptr;
	D3D11_TEXTURE2D_DESC depthSurface;
	{
		ZeroStruct(depthSurface);
		depthSurface.Width = width;
		depthSurface.Height = height;
		depthSurface.MipLevels = 1;
		depthSurface.ArraySize = 1;
		depthSurface.Format = DXGI_FORMAT_D32_FLOAT;
		depthSurface.SampleDesc.Count = 1;
		depthSurface.SampleDesc.Quality = 0;
		depthSurface.Usage = D3D11_USAGE_DEFAULT;
		depthSurface.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	}
	hr = iDevice->CreateTexture2D(&depthSurface, nullptr, &pDSBuffer);
	if (FAILED(hr)) return hr;
	//Creating DSState
	D3D11_DEPTH_STENCIL_DESC depthStencil;
	{
		ZeroStruct(depthStencil);
		depthStencil.DepthEnable = true;
		depthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		depthStencil.StencilEnable = true;
		depthStencil.StencilReadMask = 0xFF;
		depthStencil.StencilWriteMask = 0xFF;
		depthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	}
	hr = iDevice->CreateDepthStencilState(&depthStencil, &iDSState);
	if (FAILED(hr)) return hr;
	iImmediateContext->OMSetDepthStencilState(iDSState, 1);
	//Creting DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	{
		ZeroStruct(depthStencilViewDesc);
		depthStencilViewDesc.Format = depthSurface.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
	}
	hr = iDevice->CreateDepthStencilView(pDSBuffer, &depthStencilViewDesc, &iDSV);
	if (FAILED(hr)) return hr;
	iImmediateContext->OMSetRenderTargets(1, &iRTV.p, iDSV);

	D3D11_BLEND_DESC blendStateDesc;
	{
		ZeroStruct(blendStateDesc);
		blendStateDesc.AlphaToCoverageEnable = 0;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	hr = iDevice->CreateBlendState(&blendStateDesc, &iBlendState);
	if (FAILED(hr)) return hr;
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	iImmediateContext->OMSetBlendState(iBlendState, blendFactor, 0xffffffff);

	//Setting rasterizer states
	///solid
	D3D11_RASTERIZER_DESC rasterizerState;
	{
		ZeroStruct(rasterizerState);
		rasterizerState.FillMode = D3D11_FILL_SOLID;
		rasterizerState.CullMode = D3D11_CULL_BACK;
		rasterizerState.FrontCounterClockwise = false;
		rasterizerState.DepthBias = 0;
		rasterizerState.SlopeScaledDepthBias = 0.0f;
		rasterizerState.DepthBiasClamp = 0.0f;
		rasterizerState.DepthClipEnable = true;
		rasterizerState.ScissorEnable = false;
		rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = false;
	}
	hr = iDevice->CreateRasterizerState(&rasterizerState, &iRasterizerStateSolid);
	if (FAILED(hr)) return hr;
	///wireframe
	{
		ZeroStruct(rasterizerState);
		rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
		rasterizerState.CullMode = D3D11_CULL_BACK;
		rasterizerState.FrontCounterClockwise = false;
		rasterizerState.DepthBias = 0;
		rasterizerState.SlopeScaledDepthBias = 0.0f;
		rasterizerState.DepthBiasClamp = 0.0f;
		rasterizerState.DepthClipEnable = true;
		rasterizerState.ScissorEnable = false;
		rasterizerState.MultisampleEnable = false;
		rasterizerState.AntialiasedLineEnable = false;
	}
	hr = iDevice->CreateRasterizerState(&rasterizerState, &iRasterizerStateWireframe);
	if (FAILED(hr)) return hr;
	//Setup the viewport
	D3D11_VIEWPORT vp;
	{
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
	}
	iImmediateContext->RSSetViewports(1, &vp);

	return S_OK;
}

HRESULT PlanetAndMan::InitSharedResources() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SAMPLER_DESC tex2DSamplerDesc;

	//ViewProjLightConstantBuffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(ViewProjLightConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	}
	hr = iDevice->CreateBuffer(&bufferDesc, nullptr, &iViewProjLightConstantBuffer);
	if (FAILED(hr)) return hr;
	//ViewProjLightConstantBuffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(PlanetConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	}
	hr = iDevice->CreateBuffer(&bufferDesc, nullptr, &iPlanetConstantBuffer);
	if (FAILED(hr)) return hr;
	//Texture sampler
	{
		ZeroStruct(tex2DSamplerDesc);
		tex2DSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		tex2DSamplerDesc.AddressU = tex2DSamplerDesc.AddressV = tex2DSamplerDesc.AddressW =
			D3D11_TEXTURE_ADDRESS_WRAP;
		tex2DSamplerDesc.MinLOD = 0.f;
		tex2DSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	}
	hr = iDevice->CreateSamplerState(&tex2DSamplerDesc, &iTex2DSampler);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT PlanetAndMan::InitTerrainGeometry(){
	HRESULT hr;

	UINT width, height;
	GetClientWH(width, height);

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = ReadShaderFromFile(L"terrain_vs.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateVertexShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iTerrainVS);
	if (FAILED(hr)) return hr;
	///Vertex buffer format
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout {
		{ "CONTROL_POINT_LOCAL_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	hr = iDevice->CreateInputLayout(&layout[0], layout.size(), VecBuffer(shaderBytecode),
		VecBufferSize(shaderBytecode), &iTerrainVertexLayout);
	if (FAILED(hr)) return hr;

	//Hull shader
	hr = ReadShaderFromFile(L"terrain_hs.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateHullShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iTerrainHS);
	if (FAILED(hr)) return hr;
	///HS constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(TerrainHSConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
	}
	hr = iDevice->CreateBuffer(&bufferDesc, nullptr, &iTerrainHSConstantBuffer);
	if (FAILED(hr)) return hr;

	//Domain shader
	hr = ReadShaderFromFile(L"terrain_ds.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateDomainShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iTerrainDS);
	if (FAILED(hr)) return hr;
	///DS Perlin constant buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(TerrainDSPerlinConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
	}
	hr = iDevice->CreateBuffer(&bufferDesc, nullptr, &iTerrainDSPerlinConstantBuffer);
	if (FAILED(hr)) return hr;

	//Pixel shader
	hr = ReadShaderFromFile(L"terrain_ps.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreatePixelShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iTerrainPS);
	if (FAILED(hr)) return hr;

	hr = GenerateTerrain();
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT PlanetAndMan::GenerateTerrain(){
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_TEXTURE1D_DESC tex1dDesc;
	D3D11_SUBRESOURCE_DATA InitData;
	D3D11_SAMPLER_DESC LTSamplerDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	D3DX11_IMAGE_LOAD_INFO loadInfo;

	terrain = new TrianSphere(PLANET_RAD);
	terrain->divide(INITIAL_DIVISION_DEPTH);
	terrain->updatePlanesAmount();
	terrain->rebuildCPIndicesBuffer();

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(terrain->getVertices());
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	InitData.pSysMem = VecBuffer(terrain->getVertices());
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &iTerrainVertexBuffer);
	if (FAILED(hr)) return hr;

	//Setting index buffer
	{
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(terrain->getIndices());
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	InitData.pSysMem = VecBuffer(terrain->getIndices());
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &iTerrainIndexBuffer);
	if (FAILED(hr)) return hr;

	//Random permutations vector for Perlin Noise
	perlinPermutations.resize(512);
	for (int i = 0; i < 256; i++) {
		perlinPermutations[i] = i;
	}
	std::random_shuffle(perlinPermutations.begin(), perlinPermutations.begin() + 256);
	VecDuplicateLeftHalf(perlinPermutations);
	{
		ZeroStruct(tex1dDesc);
		tex1dDesc.Usage = D3D11_USAGE_IMMUTABLE;
		tex1dDesc.Width = 512;
		tex1dDesc.ArraySize = 1;
		tex1dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex1dDesc.Format = DXGI_FORMAT_R32_SINT;
		tex1dDesc.MipLevels = 1;
	}
	InitData.pSysMem = VecBuffer(perlinPermutations);
	CComPtr<ID3D11Texture1D> pPerlinPermutTex = nullptr;
	hr = iDevice->CreateTexture1D(&tex1dDesc, &InitData, &pPerlinPermutTex);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateShaderResourceView(pPerlinPermutTex, nullptr, &iPerlinPermutSRV);
	if (FAILED(hr)) return hr;

	//Perlin noise parameters
	perlinParams.PContinents		= { 4.f, 1.f, 4.1f, 0.0f }; 
	perlinParams.PVallMount			= { 1.f, 4.f, 2.1f, 0.0f };
	perlinParams.PPlainHills		= { 1.f, 4.f, 2.1f, 0.0f };
	perlinParams.PErosRidges		= { 1.f, 4.f, 2.1f, 0.0f };
	perlinParams.PHills				= { 2.f, 8.f, 5.1f, 0.0f };
	perlinParams.PErosion			= { 3.5f, 8.f, 5.1f, 0.0f };
	perlinParams.PRidges			= { 4.5f, 8.f, 5.1f, 0.0f };
	perlinParams.PVallMountThreshold		= { -0.1f, 0.4f };
	perlinParams.PPlainHillsThreshold		= { -0.48f, 0.25f };
	perlinParams.PErosRidgesThreshold		= { -0.16f, 0.32f };
	perlinParams.PMountHeightThreshold		= { -0.125f, 0.375f };

	//Textures
	{
		ZeroStruct(loadInfo);
		loadInfo.Width = loadInfo.Height = 512;
		loadInfo.Depth = D3DX11_FROM_FILE;
		loadInfo.FirstMipLevel = 0; loadInfo.MipLevels = 10;
		loadInfo.Usage = D3D11_USAGE_STAGING; loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ; 
		loadInfo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; 
		loadInfo.MipFilter = loadInfo.Filter = D3DX11_FILTER_TRIANGLE;
		loadInfo.pSrcInfo = nullptr;
	}
	std::vector<WCHAR*> terrainTextureFiles = {
		L"textures/terrain/seafloor001.jpg",
		L"textures/terrain/sand001.jpg",
		L"textures/terrain/grass001.jpg",
		L"textures/terrain/dirt001.jpg",
		L"textures/terrain/stone001.jpg",
		L"textures/terrain/snow001.jpg",
	};
	hr = CreateTexture2DArraySRV(iDevice, iImmediateContext, terrainTextureFiles, &loadInfo, &iTerrainTexturesSRV);
	if (FAILED(hr)) return hr;
	///Layers
	std::vector<TextureLayer>  textureLayers{
		TextureLayer{ { -10.f, 0.1f, 0.f } },
		TextureLayer{ { 0.1f, 0.25f, 0.1f } },
		TextureLayer{ { 0.25f, 1.25f, 0.2f } },
		TextureLayer{ { 1.25f, 2.25f, 0.2f } },
		TextureLayer{ { 2.25f, 3.0f, 0.2f } },
		TextureLayer{ { 3.0f, 10.f, 0.5f } }
	};
	{
		ZeroStruct(bufferDesc);
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.ByteWidth = VecBufferSize(textureLayers);
		bufferDesc.StructureByteStride = VecElementSize(textureLayers);
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	}
	InitData.pSysMem = VecBuffer(textureLayers);
	CComPtr<ID3D11Buffer> pTextureLayersBuffer = nullptr;
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &pTextureLayersBuffer);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateShaderResourceView(pTextureLayersBuffer, nullptr, &iTerrainTextureLayersSRV);
	if (FAILED(hr)) return hr;

	//Quintic smooth lookup table
	std::vector<float> quinticLT(512);
	for (int i = 0; i < 256; i++) {
		quinticLT[i] = quinticSmooth(i / 255.f);
	}
	for (int i = 0; i < 256; i++) { //derivative
		quinticLT[256 + i] = quinticSmoothDeriv(i / 255.f);
	}
	{
		ZeroStruct(tex1dDesc);
		tex1dDesc.Usage = D3D11_USAGE_IMMUTABLE	;
		tex1dDesc.Width = 256;
		tex1dDesc.ArraySize = 2;
		tex1dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		tex1dDesc.Format = DXGI_FORMAT_R32_FLOAT;
		tex1dDesc.MipLevels = 1;
	}
	D3D11_SUBRESOURCE_DATA InitDataArr2[2];
	InitDataArr2[0].pSysMem = VecBuffer(quinticLT);
	InitDataArr2[1].pSysMem = VecBufferFrom(quinticLT, 256);
	CComPtr<ID3D11Texture1D> pQuinticSmoothLTTex = nullptr;
	hr = iDevice->CreateTexture1D(&tex1dDesc, InitDataArr2, &pQuinticSmoothLTTex);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateShaderResourceView(pQuinticSmoothLTTex, nullptr, &iQuinticSmoothLTSRV);
	if (FAILED(hr)) return hr;

	{
		ZeroStruct(LTSamplerDesc);
		LTSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		LTSamplerDesc.AddressU = LTSamplerDesc.AddressV = LTSamplerDesc.AddressW =
			D3D11_TEXTURE_ADDRESS_CLAMP;
		LTSamplerDesc.MinLOD = 0.f;
		LTSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	}
	hr = iDevice->CreateSamplerState(&LTSamplerDesc, &iLTSampler);
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT PlanetAndMan::InitWaterGeometry() {
	HRESULT hr;

	UINT width, height;
	GetClientWH(width, height);

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = ReadShaderFromFile(L"water_vs.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateVertexShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iWaterVS);
	if (FAILED(hr)) return hr;
	///Vertex buffer format
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = iDevice->CreateInputLayout(layout, numElements, VecBuffer(shaderBytecode), 
		VecBufferSize(shaderBytecode), &iWaterVertexLayout);
	if (FAILED(hr)) return hr;

	//Pixel shader
	hr = ReadShaderFromFile(L"water_ps.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreatePixelShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iWaterPS);
	if (FAILED(hr)) return hr;

	hr = GenerateWater();
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT PlanetAndMan::GenerateWater() {
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA InitData;

	std::vector<SimpleVertex> allTriangleVertices;

	water = new TrianSphere(PLANET_RAD);
	water->divide(INITIAL_DIVISION_DEPTH);
	water->updatePlanesAmount();
	water->rebuildVertexIndicesBuffer();
	water->fillAllTriangleVertices(allTriangleVertices);

	//Setting vertex buffer
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(allTriangleVertices);
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	InitData.pSysMem = VecBuffer(allTriangleVertices);
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &iWaterVertexBuffer);
	if (FAILED(hr)) return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(iDevice, L"water.dds", nullptr, nullptr, &iWaterTextureSRV, nullptr);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT PlanetAndMan::InitManGeometry()
{
	HRESULT hr;

	UINT width, height;
	GetClientWH(width, height);

	std::vector<char> shaderBytecode;

	//Vertex shader
	hr = ReadShaderFromFile(L"man_vs.cso", shaderBytecode);
	if (FAILED(hr)) return hr;
	hr = iDevice->CreateVertexShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iCharacterVS);
	if (FAILED(hr)) return hr;
	///Vertex buffer format
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);
	hr = iDevice->CreateInputLayout(layout, numElements, VecBuffer(shaderBytecode),
		VecBufferSize(shaderBytecode), &iCharacterVertexLayout);
	if (FAILED(hr)) return hr;
	///VS constant buffer
	D3D11_BUFFER_DESC bufferDesc;
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(CharacterVSConstantBuffer);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	}
	hr = iDevice->CreateBuffer(&bufferDesc, nullptr, &iCharacterVSConstantBuffer);
	if (FAILED(hr)) return hr;

	//Pixel shader
	ReadShaderFromFile(L"man_ps.cso", shaderBytecode);
	hr = iDevice->CreatePixelShader(VecBuffer(shaderBytecode), VecBufferSize(shaderBytecode), nullptr, &iCharacterPS);
	if (FAILED(hr)) return hr;

	hr = GenerateMan();
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT PlanetAndMan::GenerateMan()
{
	HRESULT hr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA InitData;

	man = new Character(this);

	//Vertex buffer
	std::vector<ColoredVertex> vertices = {
		{ { -0.02f, 0.f, -0.02f },{ 0.f, 0.f, 1.f } },
		{ { 0.02f, 0.f, -0.02f },{ 0.f, 0.f, 1.f } },
		{ { 0.f, 0.01f, -0.01f },{ 1.f, 1.f, 1.f } },
		{ { 0.f, 0.f, 0.02f },{ 1.f, 0.f, 0.f } }
	};
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(vertices);
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	}
	InitData.pSysMem = VecBuffer(vertices);
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &iCharacterVertexBuffer);
	if (FAILED(hr)) return hr;

	//Index buffer
	std::vector<DWORD> indices = {
		0, 2, 1,
		3, 2, 0,
		1, 2, 3,
		1, 3, 0
	};
	{
		ZeroStruct(bufferDesc);
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = VecBufferSize(indices);
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	}
	InitData.pSysMem = VecBuffer(indices);
	hr = iDevice->CreateBuffer(&bufferDesc, &InitData, &iCharacterIndexBuffer);
	if (FAILED(hr)) return hr;

	return S_OK;
}

HRESULT PlanetAndMan::ReadShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode){
	std::ifstream input(szFileName, std::ios::binary);
	shaderBytecode = std::vector<char>(std::istreambuf_iterator<char>(input),
		(std::istreambuf_iterator<char>())); //without the parentheses, the compiler treats it as a function declaration
	return (shaderBytecode.size() == 0 ? E_FAIL : S_OK);
}

HRESULT PlanetAndMan::CreateTexture2DArraySRV(ID3D11Device* device, ID3D11DeviceContext* context, 
	const std::vector<WCHAR*>& filenames, D3DX11_IMAGE_LOAD_INFO* textureInfo, ID3D11ShaderResourceView** srv){
	HRESULT hr;

	D3D11_TEXTURE2D_DESC texArrayDesc;
	//D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	std::vector<CComPtr<ID3D11Texture2D>> srcTex(filenames.size());

	for (int i = 0; i < filenames.size(); i++) {
		hr = D3DX11CreateTextureFromFile(device, filenames[i], textureInfo, nullptr, (ID3D11Resource**)&srcTex[i], nullptr); 
		if (FAILED(hr)) return hr;
	}

	{
		ZeroStruct(texArrayDesc);
		texArrayDesc.Width = textureInfo->Width;
		texArrayDesc.Height = textureInfo->Height;
		texArrayDesc.MipLevels = textureInfo->MipLevels;
		texArrayDesc.ArraySize = filenames.size();
		texArrayDesc.Format = textureInfo->Format;
		texArrayDesc.SampleDesc.Count = 1;
		texArrayDesc.SampleDesc.Quality = 0;
		texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
		texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	}
	CComPtr<ID3D11Texture2D> texArray = nullptr; 
	hr = device->CreateTexture2D(&texArrayDesc, nullptr, &texArray);
	if (FAILED(hr)) return hr;

	for (int tex = 0; tex < texArrayDesc.ArraySize; tex++) {
		for (int mip = 0; mip < texArrayDesc.MipLevels; mip++) {
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			hr = context->Map(srcTex[tex], mip, D3D11_MAP_READ, 0, &mappedTex2D);
			if (FAILED(hr)) return hr;
			context->UpdateSubresource(
				texArray,
				D3D11CalcSubresource(mip, tex, textureInfo->MipLevels),
				nullptr,
				mappedTex2D.pData,
				mappedTex2D.RowPitch,
				mappedTex2D.DepthPitch);
			context->Unmap(srcTex[tex], mip);
		}
	}

	hr = device->CreateShaderResourceView(texArray, nullptr, srv);
	if (FAILED(hr)) return hr;

	return S_OK;
}

void PlanetAndMan::GetClientWH(UINT &width, UINT &height){
	RECT rc;
	GetClientRect(hWindow, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

Camera* PlanetAndMan::getCamera(){
	return pCamera;
}

Character* PlanetAndMan::getMan(){
	return man;
}

void PlanetAndMan::switchCamera(){
	bCameraPlanetOrMan ^= true;
}

void PlanetAndMan::switchRender() {
	bRenderPlanetOrMan ^= true;
}

void PlanetAndMan::switchRasterizer() {
	bSolidOrWireframe ^= true;
}

float PlanetAndMan::perlinNoise(XMFLOAT3 loc) {
	UINT3 locInt{ (int)floor(loc.x) & 255, (int)floor(loc.y) & 255, (int)floor(loc.z) & 255 };
	float x = loc.x - (float)locInt.x; float x1 = x - 1.f;
	float y = loc.y - (float)locInt.y; float y1 = y - 1.f;
	float z = loc.z - (float)locInt.z; float z1 = z - 1.f;

		//hash coordinates
	int A = perlinPermutations[locInt.x] + locInt.y,
		AA = perlinPermutations[A] + locInt.z,
		AAA = perlinPermutations[AA],
		AAB = perlinPermutations[AA + 1],
		AB = perlinPermutations[A + 1] + locInt.z,
		ABA = perlinPermutations[AB],
		ABB = perlinPermutations[AB + 1],
		B = perlinPermutations[locInt.x + 1] + locInt.y,
		BA = perlinPermutations[B] + locInt.z,
		BAA = perlinPermutations[BA],
		BAB = perlinPermutations[BA + 1],
		BB = perlinPermutations[B + 1] + locInt.z,
		BBA = perlinPermutations[BB],
		BBB = perlinPermutations[BB + 1];

	float sx = quinticSmooth(x);
	float sy = quinticSmooth(y);
	float sz = quinticSmooth(z);

	float g000 = gridGrad(AAA, x, y, z);
	float g100 = gridGrad(BAA, x1, y, z);
	float g010 = gridGrad(ABA, x, y1, z);
	float g110 = gridGrad(BBA, x1, y1, z);
	float g001 = gridGrad(AAB, x, y, z1);
	float g101 = gridGrad(BAB, x1, y, z1);
	float g011 = gridGrad(ABB, x, y1, z1);
	float g111 = gridGrad(BBB, x1, y1, z1);

	float k100 = g100 - g000;
	float k010 = g010 - g000;
	float k001 = g001 - g000;
	float k110 = g110 - g010 - g100 + g000;
	float k101 = g101 - g001 - g100 + g000;
	float k011 = g011 - g001 - g010 + g000;
	float k111 = g111 - g011 - g101 + g001 - g110 + g010 + g100 - g000;

	return g000
		+ sx*k100 + sy*k010 + sz*k001
		+ sx*sy*k110
		+ sx*sz*k101
		+ sy*sz*k011
		+ sx*sy*sz*k111;
}

float PlanetAndMan::getPerlinHeight(XMFLOAT3 normal) {
	float finalHeight = 0;
	float octave, amplitude; int octavesN;
	amplitude = perlinParams.PContinents.x; octave = perlinParams.PContinents.y; octavesN = (int)perlinParams.PContinents.z;
	for (int i = 0; i < octavesN; i++) {
		finalHeight += perlinNoise(XMFLOAT3{
			(normal.x + 1.f) * octave + perlinParams.PContinents.w ,
			(normal.y + 1.f) * octave + perlinParams.PContinents.w ,
			(normal.z + 1.f) * octave + perlinParams.PContinents.w
		}) * amplitude;
		octave *= 2.f; 
		amplitude /= 2.f;
	}
	float valleysOrMountains = 0.f;
	amplitude = perlinParams.PVallMount.x; octave = perlinParams.PVallMount.y; octavesN = (int)perlinParams.PVallMount.z;
	for (int i = 0; i < octavesN; i++) {
		valleysOrMountains += perlinNoise(XMFLOAT3{
			(normal.x + 1.f) * octave + perlinParams.PVallMount.w,
			(normal.y + 1.f) * octave + perlinParams.PVallMount.w,
			(normal.z + 1.f) * octave + perlinParams.PVallMount.w
		}) * amplitude;
		octave *= 2.f;
		amplitude /= 2.f;
	}
	valleysOrMountains = clamp((valleysOrMountains - perlinParams.PVallMountThreshold.x) / perlinParams.PVallMountThreshold.y, 0.f, 1.f);
	if (valleysOrMountains < 1.f - EPS) {
		float plainsOrHills = 0.f;
		amplitude = perlinParams.PPlainHills.x;  octave = perlinParams.PPlainHills.y; octavesN = (int)perlinParams.PPlainHills.z;
		for (int i = 0; i < octavesN; i++) {
			plainsOrHills += perlinNoise(XMFLOAT3{
				(normal.x + 1.f) * octave + perlinParams.PPlainHills.w,
				(normal.y + 1.f) * octave + perlinParams.PPlainHills.w,
				(normal.z + 1.f) * octave + perlinParams.PPlainHills.w
			}) * amplitude;
			octave *= 2.f;
			amplitude /= 2.f;
		}
		plainsOrHills = clamp((plainsOrHills - perlinParams.PPlainHillsThreshold.x) / perlinParams.PPlainHillsThreshold.y, 0.f, 1.f);
		if (plainsOrHills > EPS) {
			amplitude = plainsOrHills * (1.f - valleysOrMountains) * perlinParams.PHills.x;
			octave = perlinParams.PHills.y; octavesN = (int)perlinParams.PHills.z;
			for (int i = 0; i < octavesN; i++) {
				finalHeight += perlinNoise(XMFLOAT3{
					(normal.x + 1.f) * octave + perlinParams.PHills.w,
					(normal.y + 1.f) * octave + perlinParams.PHills.w,
					(normal.z + 1.f) * octave + perlinParams.PHills.w
				}) * amplitude;
				octave *= 2.f;
				amplitude /= 2.f;
			}
		}
	}
	if (valleysOrMountains > EPS) {
		float heightAllowsMount = clamp((finalHeight - perlinParams.PMountHeightThreshold.x) / perlinParams.PMountHeightThreshold.y, 0.f, 1.f);
		if (heightAllowsMount > EPS) {
			float erosionOrRidges = 0.f;
			amplitude = perlinParams.PErosRidges.x; octave = perlinParams.PErosRidges.y; octavesN = (int)perlinParams.PErosRidges.z;
			for (int i = 0; i < octavesN; i++) {
				erosionOrRidges += perlinNoise(XMFLOAT3{
					(normal.x + 1.f) * octave + perlinParams.PErosRidges.w,
					(normal.y + 1.f) * octave + perlinParams.PErosRidges.w,
					(normal.z + 1.f) * octave + perlinParams.PErosRidges.w
				}) * amplitude;
				octave *= 2.f;
				amplitude /= 2.f;
			}
			erosionOrRidges = clamp((erosionOrRidges - perlinParams.PErosRidgesThreshold.x) / perlinParams.PErosRidgesThreshold.y, 0.f, 1.f);;
			if (erosionOrRidges < 1.f - EPS) {
				amplitude = (1.f - erosionOrRidges) * heightAllowsMount * valleysOrMountains * perlinParams.PErosion.x;
				octave = perlinParams.PErosion.y; octavesN = (int)perlinParams.PErosion.z;
				for (int i = 0; i < octavesN; i++) {
					finalHeight += abs(perlinNoise(XMFLOAT3{
						(normal.x + 1.f) * octave + perlinParams.PErosion.w,
						(normal.y + 1.f) * octave + perlinParams.PErosion.w,
						(normal.z + 1.f) * octave + perlinParams.PErosion.w
					}) * amplitude);
					octave *= 2.f;
					amplitude /= 2.f;
				}
			}
			if (erosionOrRidges > EPS) {
				amplitude = erosionOrRidges * heightAllowsMount * valleysOrMountains * perlinParams.PRidges.x;
				octave = perlinParams.PRidges.y; octavesN = (int)perlinParams.PRidges.z;
				for (int i = 0; i < octavesN; i++) {
					finalHeight += 0.32 * amplitude - abs(perlinNoise(XMFLOAT3{
						(normal.x + 1.f) * octave + perlinParams.PRidges.w,
						(normal.y + 1.f) * octave + perlinParams.PRidges.w,
						(normal.z + 1.f) * octave + perlinParams.PRidges.w
					}) * amplitude);
					octave *= 2.f;
					amplitude /= 2.f;
				}
			}
		}
	}
	return finalHeight * 0.1f;
}

ControlsApplier* PlanetAndMan::getControlsApplier(){
	return pControlsApplier;
}

PlanetAndMan::~PlanetAndMan(){
	if (iImmediateContext) iImmediateContext->ClearState();
	delete man;
	delete pCamera;
	delete pControlsApplier;
}