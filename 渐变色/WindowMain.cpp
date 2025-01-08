#include "WindowMain.h"
using namespace Microsoft::WRL;

WindowMain::WindowMain()
{
    createWindow();
    initD2D();
}

WindowMain::~WindowMain()
{
    compositionVisual.Reset();
    compositionTarget.Reset();
    compositionDevice.Reset();
    renderTarget.Reset();
    d2dFactory.Reset();
    swapChain.Reset();
    dxgiDevice.Reset();
    d3dDevice.Reset();
}

void WindowMain::show()
{
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void WindowMain::createWindow()
{
    auto hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = hInstance;
    wc.lpszClassName = L"D2DWin";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = routeWinMsg;
    RegisterClass(&wc);
    hwnd = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP, wc.lpszClassName, L"Sample", WS_OVERLAPPEDWINDOW, x, y, w, h, nullptr, nullptr, hInstance, nullptr);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

void WindowMain::initD2D()
{
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT, NULL, 0, D3D11_SDK_VERSION, &d3dDevice, NULL, NULL);
    d3dDevice.As(&dxgiDevice);
    ComPtr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory2(0, __uuidof(dxgiFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));

    DXGI_SWAP_CHAIN_DESC1 description = {};
    description.Width = w;
    description.Height = h;
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount = 2;
    description.SampleDesc.Count = 1;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
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
}

void WindowMain::paint()
{
    renderTarget->BeginDraw();
    renderTarget->Clear(D2D1::ColorF(0.6f, 0.6f, 0.2f, 0.5f));
    {
        D2D1_GRADIENT_STOP gradientStops[2];
        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Red);
        gradientStops[0].position = 0.0f; // 起始点颜色
        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Blue);
        gradientStops[1].position = 1.0f; // 终点颜色
        ComPtr<ID2D1GradientStopCollection> gradientStopCollection;
        renderTarget->CreateGradientStopCollection(gradientStops,2,&gradientStopCollection);
        ComPtr<ID2D1LinearGradientBrush> linearGradientBrush;
        renderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0),D2D1::Point2F(200, 200)),
            gradientStopCollection.Get(),
            &linearGradientBrush
        );
        renderTarget->FillRectangle(D2D1::RectF(50, 50, 250, 250), linearGradientBrush.Get());
    }
    {
        D2D1_GRADIENT_STOP gradientStops[2];
        gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::Green);
        gradientStops[0].position = 0.0f; // 中心点颜色
        gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::Yellow);
        gradientStops[1].position = 1.0f; // 外边界颜色 
        ComPtr<ID2D1GradientStopCollection> gradientStopCollection;
        renderTarget->CreateGradientStopCollection(gradientStops,2,&gradientStopCollection); 
        ComPtr<ID2D1RadialGradientBrush> radialGradientBrush;
        renderTarget->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
                D2D1::Point2F(400, 150),  // 中心点
                D2D1::Point2F(0, 0),     // 偏移量
                100, 100
            ),
            gradientStopCollection.Get(),
            &radialGradientBrush
        );
        renderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(400, 150), 100, 100), radialGradientBrush.Get());
    }


    renderTarget->EndDraw();
    swapChain->Present(1, 0);
}

void WindowMain::resetRenderTarget()
{
    if (w <= 0 || h <= 0)return;
    renderTarget.Reset();
    swapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    ComPtr<IDXGISurface2> dxgiSurface;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
    D2D1_RENDER_TARGET_PROPERTIES targetProperties = {};
    targetProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    targetProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
    targetProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.Get(), &targetProperties, &renderTarget);
}

LRESULT WindowMain::routeWinMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto obj = reinterpret_cast<WindowMain*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!obj) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    if (msg == WM_DESTROY) {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, NULL);
        UnregisterClass(L"D2DWin", NULL);
        PostQuitMessage(0);
        return 0;
    }
    return obj->processWinMsg(msg, wParam, lParam);
}

LRESULT WindowMain::processWinMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paint();
        EndPaint(hwnd, &ps);
        return 0;
    }
    else if(msg == WM_MOVE)
    {
        x = LOWORD(lParam);
        y = HIWORD(lParam);
        return 0;
    }
    else if (msg == WM_SIZE) {
        w = LOWORD(lParam);
        h = HIWORD(lParam);
        resetRenderTarget();
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}