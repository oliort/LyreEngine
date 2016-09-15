#include <windows.h>
#include <xnamath.h>
#include <time.h>

#pragma enable_d3d11_debug_symbols

#include "PlanetAndMan.h"
#include "Resource.h"
#include "Camera.h"
#include "KeyTranslatorDefault.h"

KeyTranslator* translator = nullptr;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	case WM_KEYDOWN:
		if (translator != nullptr)
			translator->press(wParam);
		break;
	case WM_KEYUP:
		if (translator != nullptr)
			translator->release(wParam);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow){
	srand(time(0));
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	PlanetAndMan app(hInstance, nCmdShow, MsgProc);
	ControlsApplier* applier = app.getControlsApplier();
	translator = new KeyTranslatorDefault(applier);
	MSG msg = {0};
	while (WM_QUIT != msg.message){
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			applier->processControls();
			app.Render();
		}
	}
	return (int)msg.wParam;
}
