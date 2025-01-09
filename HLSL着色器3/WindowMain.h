#pragma once
#include <windows.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#include <wincodec.h>
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
	int x{ 100 }, y{ 100 }, w{ 800 }, h{ 600 };
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID2D1Factory2> d2dFactory;
	ComPtr<ID2D1Device> d2dDevice;
	ComPtr<ID2D1DeviceContext> d2dContext;
	ComPtr<ID2D1Bitmap1> renderBitmap;
	ComPtr<IDCompositionDevice> compositionDevice;
	ComPtr<IDCompositionTarget> compositionTarget;
	ComPtr<IDCompositionVisual> compositionVisual;
};

