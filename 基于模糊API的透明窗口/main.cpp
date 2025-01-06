#include <d2d1.h>
#include <dwmapi.h>
#include <versionhelpers.h>

static int w{ 800 }, h{600};
static ID2D1Factory* d2d1Factory = nullptr;
static ID2D1HwndRenderTarget* renderTarget = nullptr;

bool EnableAlphaCompositing(HWND hWnd)
{
    if (!IsWindowsVistaOrGreater()) { return false; }
    BOOL isCompositionEnable = false;
    //检查DWM是否启用
    DwmIsCompositionEnabled(&isCompositionEnable);
    if (!isCompositionEnable) { return true; }
    DWORD currentColor = 0;
    BOOL isOpaque = false;
    //检查是否支持毛玻璃效果
    DwmGetColorizationColor(&currentColor, &isOpaque);
    if (!isOpaque || IsWindows8OrGreater())
    {
        HRGN region = CreateRectRgn(0, 0, -1, -1);
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = region;
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(hWnd, &bb);
        DeleteObject(region);
        return true;
    }
    else // For Window7
    {
        DWM_BLURBEHIND bb = { 0 };
        bb.dwFlags = DWM_BB_ENABLE;
        DwmEnableBlurBehindWindow(hWnd, &bb);
        return false;
    }
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        renderTarget->BeginDraw();
        renderTarget->Clear(D2D1::ColorF(0.2f, 0.3f, 0.5f, 0.5f));
        renderTarget->EndDraw();
        EndPaint(hWnd, &ps);
        return 0;
    }
    else if (uMsg == WM_SIZING) {
        RECT* pRect = reinterpret_cast<RECT*>(lParam);
        w = pRect->right - pRect->left;
        h = pRect->bottom - pRect->top;
        auto size = D2D1::SizeU(w, h);
        renderTarget->Resize(size);
        return 0;
    }
    else if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASSEXW wcx = {};
    wcx.cbSize = sizeof(wcx);
    wcx.lpfnWndProc = WindowProc;
    wcx.lpszClassName = L"Direct2DSample";
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wcx);
    HWND hwnd = CreateWindowEx(NULL, wcx.lpszClassName, L"SampleWindow", WS_POPUP,
        200, 200, w, h,nullptr, nullptr, hInstance, nullptr);
    EnableAlphaCompositing(hwnd);
    {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,&d2d1Factory);
        auto pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
        auto renderProps = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,pixelFormat);
        auto size = D2D1::SizeU(w, h);
        auto hwndRenderProps = D2D1::HwndRenderTargetProperties(hwnd, size);
        d2d1Factory->CreateHwndRenderTarget(renderProps, hwndRenderProps, &renderTarget);        
    }
    ShowWindow(hwnd, SW_NORMAL);
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    renderTarget->Release();
    d2d1Factory->Release();
    UnregisterClass(wcx.lpszClassName, hInstance);
}