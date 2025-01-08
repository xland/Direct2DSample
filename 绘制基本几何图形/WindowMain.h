#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
using namespace Microsoft::WRL;

class WindowMain
{
public:
	WindowMain();
	~WindowMain();
	void show();
private:
	void createWindow();
	void initD2D();
	void paint();
	void resetRenderTarget();
	static LRESULT CALLBACK routeWinMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK processWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	HWND hwnd;
	int x{ 100 }, y{ 100 }, w{ 800 }, h{ 1200 };
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID2D1Factory2> d2dFactory;
	ComPtr<ID2D1RenderTarget> renderTarget;
	ComPtr<IDCompositionDevice> compositionDevice;
	ComPtr<IDCompositionTarget> compositionTarget;
	ComPtr<IDCompositionVisual> compositionVisual;
};

