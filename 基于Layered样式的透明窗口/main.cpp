#include <Windows.h>
#include <windowsx.h>
#include <blend2d.h>

static int w{ 800 }, h{600};
static HWND hwnd;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    default:
    {
        break;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void paint()
{
    BLImage img(w, h, BL_FORMAT_PRGB32);
    BLContext ctx(img);
    ctx.fillAll(BLRgba32(0x22B83E52));
    BLImageData data;
    img.getData(&data);

    HDC hdc = GetDC(hwnd);
    auto compDC = CreateCompatibleDC(hdc);
    auto bitmap = CreateCompatibleBitmap(hdc, w, h);
    DeleteObject(SelectObject(compDC, bitmap));

    BITMAPINFO info = { sizeof(BITMAPINFOHEADER), w, 0 - h, 1, 32, BI_RGB, w * 4 * h, 0, 0, 0, 0 };
    SetDIBits(hdc, bitmap, 0, h, data.pixelData, &info, DIB_RGB_COLORS);
    BLENDFUNCTION blend = { .BlendOp{AC_SRC_OVER}, .SourceConstantAlpha{255}, .AlphaFormat{AC_SRC_ALPHA} };
    POINT pSrc = { 0, 0 };
    SIZE sizeWnd = { w, h };
    UpdateLayeredWindow(hwnd, hdc, NULL, &sizeWnd, compDC, &pSrc, NULL, &blend, ULW_ALPHA);
    ReleaseDC(hwnd, hdc);

    DeleteDC(compDC);
    DeleteObject(bitmap);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASSEX wcx{};
    wcx.cbSize = sizeof(wcx);
    wcx.lpfnWndProc = WindowProc;
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.hInstance = hInstance;
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.lpszClassName = L"LayeredWindow";
    RegisterClassEx(&wcx);
    hwnd = CreateWindowEx(WS_EX_LAYERED, wcx.lpszClassName, wcx.lpszClassName, WS_POPUP, 100, 100, w, h, NULL, NULL, hInstance, NULL);
    paint();
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterClass(wcx.lpszClassName, hInstance);
}