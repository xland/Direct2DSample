#include <windows.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
using namespace Microsoft::WRL;

HWND hwnd;
int x{ 100 },y{100},w{ 800 }, h{ 600 };
ComPtr<ID3D11Device> d3dDevice;
ComPtr<IDXGIDevice> dxgiDevice;
ComPtr<IDXGISwapChain1> swapChain;
ComPtr<ID2D1Factory2> d2dFactory;
ComPtr<ID2D1RenderTarget> renderTarget;
ComPtr<IDCompositionDevice> compositionDevice;
ComPtr<IDCompositionTarget> compositionTarget;
ComPtr<IDCompositionVisual> compositionVisual;
ComPtr<ID2D1SolidColorBrush> brush;


void initD2D() {
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, 0, D3D11_SDK_VERSION, &d3dDevice, NULL, NULL);
    d3dDevice.As(&dxgiDevice);
    ComPtr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxgiFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    DXGI_SWAP_CHAIN_DESC1 description = {};
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount = 2;
    description.SampleDesc.Count = 1;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    description.Width = w;
    description.Height = h;
    dxgiFactory->CreateSwapChainForComposition(dxgiDevice.Get(), &description, nullptr, swapChain.GetAddressOf());
    dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    ComPtr<IDXGISurface2> dxgiSurface;
    swapChain->GetBuffer(0, __uuidof(dxgiSurface), reinterpret_cast<void**>(dxgiSurface.GetAddressOf()));

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf());
    
    D2D1_RENDER_TARGET_PROPERTIES targetProperties = {};
    targetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    targetProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
    targetProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.Get(), &targetProperties, &renderTarget);

    DCompositionCreateDevice(dxgiDevice.Get(), IID_PPV_ARGS(&compositionDevice));
    compositionDevice->CreateTargetForHwnd(hwnd, true, &compositionTarget);
    compositionDevice->CreateVisual(&compositionVisual);
    compositionVisual->SetContent(swapChain.Get());
    compositionTarget->SetRoot(compositionVisual.Get());
    compositionDevice->Commit();


    D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f, 0.55f, 0.34f, 0.75f); // rgba
    renderTarget->CreateSolidColorBrush(brushColor, brush.GetAddressOf());
}



LRESULT CALLBACK winProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        renderTarget->BeginDraw();
        renderTarget->Clear();
        D2D1_POINT_2F const ellipseCenter = D2D1::Point2F(w-160, h-200); // y
        D2D1_ELLIPSE const ellipse = D2D1::Ellipse(ellipseCenter, 100.0f, 100.0f);
        renderTarget->FillEllipse(ellipse, brush.Get());
        renderTarget->EndDraw(); // Make the swap chain available to the composition engine
        swapChain->Present(1, 0);
        EndPaint(hWnd, &ps);
        return 0;
    }
    else if (uMsg == WM_SIZE) {
        if (!renderTarget) return 0;
        RECT r;
        GetWindowRect(hwnd, &r);
        x = r.left;
        y = r.top;
        w = r.right - r.left;
        h = r.bottom - r.top;
        renderTarget.Reset();
        swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        ComPtr<IDXGISurface2> dxgiSurface;
        swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
        D2D1_RENDER_TARGET_PROPERTIES targetProperties = {};
        targetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        targetProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
        targetProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.Get(), &targetProperties, &renderTarget);
        return 0;
    }
    else if (uMsg == WM_DESTROY) {
        compositionVisual.Reset();
        compositionTarget.Reset();
        compositionDevice.Reset();
        renderTarget.Reset();
        d2dFactory.Reset();
        swapChain.Reset();
        d3dDevice.Reset();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
    WNDCLASS wc = {};
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.lpszClassName = L"window";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = winProc;
    RegisterClass(&wc);
    hwnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wc.lpszClassName, L"Sample", WS_OVERLAPPEDWINDOW | WS_VISIBLE, x, y, w, h, nullptr, nullptr, hInstance, nullptr);
    initD2D();
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}