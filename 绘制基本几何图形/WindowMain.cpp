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

    ComPtr<ID2D1SolidColorBrush> brush;
    D2D1_COLOR_F brushColor = D2D1::ColorF(0.18f, 0.68f, 0.86f, 1.f);
    renderTarget->CreateSolidColorBrush(brushColor, brush.GetAddressOf());

    D2D1_STROKE_STYLE_PROPERTIES strokeprops = D2D1::StrokeStyleProperties();
    strokeprops.endCap = D2D1_CAP_STYLE_ROUND;
    strokeprops.startCap = D2D1_CAP_STYLE_ROUND;
    strokeprops.dashStyle = D2D1_DASH_STYLE::D2D1_DASH_STYLE_DASH;
    ComPtr<ID2D1StrokeStyle> strokeStyle;
    d2dFactory->CreateStrokeStyle(strokeprops, nullptr, 0, strokeStyle.GetAddressOf());


    auto ellipse0 = D2D1::Ellipse({120,120}, 100.0f, 100.0f);
    renderTarget->FillEllipse(ellipse0, brush.Get());

    auto ellipse1 = D2D1::Ellipse({ 340,120 }, 100.0f, 50.0f);
    renderTarget->DrawEllipse(ellipse1, brush.Get(), 2.f);

    auto ellipse2 = D2D1::Ellipse({ 560,120 }, 50.0f, 100.0f);
    renderTarget->DrawEllipse(ellipse2, brush.Get(), 4.f, strokeStyle.Get());

    brush->SetColor(D2D1::ColorF(0.68f, 0.18f, 0.86f, 1.f));
    auto rect0 = D2D1::RectF(20.f,240.f,220.0f, 440.0f);
    renderTarget->FillRectangle(rect0, brush.Get());

    auto rect1 = D2D1::RoundedRect(D2D1::RectF(240.f, 240.f, 440.0f, 440.0f),12.F,12.f);
    renderTarget->DrawRoundedRectangle(rect1, brush.Get(), 2.f);

    auto rect2 = D2D1::RectF(460.f, 240.f, 680.0f, 440.0f);
    renderTarget->DrawRectangle(rect2, brush.Get(), 6.f, strokeStyle.Get());

    auto strokeprops2 = D2D1::StrokeStyleProperties();
    strokeprops2.endCap = D2D1_CAP_STYLE_ROUND;
    strokeprops2.startCap = D2D1_CAP_STYLE_ROUND;
    ComPtr<ID2D1StrokeStyle> strokeStyle2;
    d2dFactory->CreateStrokeStyle(strokeprops2, nullptr, 0, strokeStyle2.GetAddressOf());
    brush->SetColor(D2D1::ColorF(0.68f, 0.26f, 0.18f, 1.f));
    renderTarget->DrawLine({ 20.f,480.f }, { 680.0f, 580.0f }, brush.Get(), 12.f, strokeStyle2.Get());
    renderTarget->DrawLine({ 20.f,580.f }, { 680.0f, 480.0f }, brush.Get(), 12.f,strokeStyle.Get());

    

    auto strokeprops3 = D2D1::StrokeStyleProperties();
    strokeprops3.lineJoin = D2D1_LINE_JOIN_ROUND;
    ComPtr<ID2D1StrokeStyle> strokeStyle3;
    d2dFactory->CreateStrokeStyle(strokeprops3, nullptr, 0, strokeStyle3.GetAddressOf());
    ComPtr<ID2D1GeometrySink> sink;
    ComPtr<ID2D1PathGeometry> pathGeometry;
    d2dFactory->CreatePathGeometry(&pathGeometry);
    pathGeometry->Open(&sink);
    sink->BeginFigure(D2D1::Point2F(160.0f, 620.0f), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(380.0f, 680.0f));
    sink->AddLine(D2D1::Point2F(260.0f, 720.0f));
    sink->AddLine(D2D1::Point2F(280.0f, 780.0f));
    sink->AddLine(D2D1::Point2F(180.0f, 800.0f));
    sink->AddLine(D2D1::Point2F(60.0f, 700.0f));
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    brush->SetColor(D2D1::ColorF(0.88f, 0.38f, 0.68f, 1.f));
    renderTarget->DrawGeometry(pathGeometry.Get(), brush.Get(), 8.0f, strokeStyle3.Get());
    

    ComPtr<ID2D1GeometrySink> sink2;
    ComPtr<ID2D1PathGeometry> pathGeometry2;
    d2dFactory->CreatePathGeometry(&pathGeometry2);
    pathGeometry2->Open(&sink2);
    sink2->BeginFigure(D2D1::Point2F(460.0f, 820.0f), D2D1_FIGURE_BEGIN_FILLED);
    sink2->AddArc(D2D1::ArcSegment(D2D1::Point2F(660.f, 680.f),
        D2D1::SizeF(20, 20),
        0.0f,
        D2D1_SWEEP_DIRECTION_CLOCKWISE,
        D2D1_ARC_SIZE_SMALL
    ));
    sink2->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink2->Close();
    brush->SetColor(D2D1::ColorF(0.48f, 0.08f, 0.98f, 1.f));
    renderTarget->DrawGeometry(pathGeometry2.Get(), brush.Get(), 8.0f, strokeStyle3.Get());


    ComPtr<ID2D1GeometrySink> sink3;
    ComPtr<ID2D1PathGeometry> pathGeometry3;
    d2dFactory->CreatePathGeometry(&pathGeometry3);
    pathGeometry3->Open(&sink3);
    sink3->BeginFigure(D2D1::Point2F(60.0f, 900.0f), D2D1_FIGURE_BEGIN_HOLLOW);
    sink3->AddBezier(D2D1::BezierSegment(
            D2D1::Point2F(360.0f, 800.0f),
            D2D1::Point2F(520.0f, 1000.0f),
            D2D1::Point2F(700.0f, 900.0f)
        ));
    sink3->EndFigure(D2D1_FIGURE_END_OPEN);
    sink3->Close();
    renderTarget->DrawGeometry(pathGeometry3.Get(), brush.Get(), 8.0f, strokeStyle2.Get());

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