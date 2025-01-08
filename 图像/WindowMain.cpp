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

    ComPtr<IWICImagingFactory> wicFactory;
    auto hr = CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&wicFactory));
    ComPtr<IWICBitmapDecoder> decoder;
    wicFactory->CreateDecoderFromFilename(L"../../../../author.jpg",
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnDemand,
        &decoder
    );
    ComPtr<IWICBitmapFrameDecode> frame;
    decoder->GetFrame(0, &frame);

    ComPtr<IWICFormatConverter> converter;
    wicFactory->CreateFormatConverter(&converter);

    converter->Initialize(
        frame.Get(),
        GUID_WICPixelFormat32bppPBGRA,  // 转换为 Direct2D 兼容格式
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );

    ComPtr<ID2D1Bitmap> d2dBitmap;
    renderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &d2dBitmap);

    D2D1_RECT_F destRect = D2D1::RectF(20, 20, 180, 180);
    renderTarget->DrawBitmap(d2dBitmap.Get(),destRect);

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