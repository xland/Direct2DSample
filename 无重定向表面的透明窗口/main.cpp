#include <windows.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
using namespace Microsoft::WRL;

ComPtr<ID3D11Device> direct3dDevice;
ComPtr<IDXGIDevice> dxgiDevice;
ComPtr<IDXGIFactory2> dxFactory;
ComPtr<IDXGISwapChain1> swapChain;

ComPtr<ID2D1Factory2> d2Factory;
ComPtr<ID2D1Device1> d2Device;
ComPtr<ID2D1DeviceContext> dc;
ComPtr<IDXGISurface2> surface;

ComPtr<IDCompositionDevice> dcompDevice;
ComPtr<IDCompositionTarget> target;
ComPtr<IDCompositionVisual> visual;

int WINAPI wWinMain(HINSTANCE module, HINSTANCE, PWSTR, int)
{
    D3D11CreateDevice(nullptr,    // Adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,    // Module
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,  //支持与Direct2D的互操作性
        nullptr, 0, // Highest available feature level
        D3D11_SDK_VERSION,
        &direct3dDevice,
        nullptr,    // Actual feature level
        nullptr);  // Device context    
    direct3dDevice.As(&dxgiDevice);
    CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,__uuidof(dxFactory),reinterpret_cast<void**>(dxFactory.GetAddressOf()));



    WNDCLASS wc = {};
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hInstance = module;
    wc.lpszClassName = L"window";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc =
        [](HWND window, UINT message, WPARAM wparam,
            LPARAM lparam) -> LRESULT
        {
            if (WM_DESTROY == message)
            {
                PostQuitMessage(0);
                return 0;
            }
            return DefWindowProc(window, message, wparam, lparam);
        };
    RegisterClass(&wc);
    HWND const window = CreateWindowEx(WS_EX_NOREDIRECTIONBITMAP,
        wc.lpszClassName, L"Sample",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 800, 600, nullptr, nullptr, module, nullptr);

    DXGI_SWAP_CHAIN_DESC1 description = {};
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount = 2;
    description.SampleDesc.Count = 1;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    RECT rect = {};
    GetClientRect(window, &rect);
    description.Width = rect.right - rect.left;
    description.Height = rect.bottom - rect.top;

    dxFactory->CreateSwapChainForComposition(dxgiDevice.Get(),
        &description,nullptr, // Don’t restrict
        swapChain.GetAddressOf());




    
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,d2Factory.GetAddressOf());
    // Create the Direct2D device that links back to the Direct3D device

    d2Factory->CreateDevice(dxgiDevice.Get(), d2Device.GetAddressOf());
    // Create the Direct2D device context that is the actual render target
    // and exposes drawing commands

    d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,dc.GetAddressOf());
    // Retrieve the swap chain's back buffer

    swapChain->GetBuffer(0,__uuidof(surface), reinterpret_cast<void**>(surface.GetAddressOf()));
    // Create a Direct2D bitmap that points to the swap chain surface
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    ComPtr<ID2D1Bitmap1> bitmap;
    dc->CreateBitmapFromDxgiSurface(surface.Get(), properties, bitmap.GetAddressOf());
    // Point the device context to the bitmap for rendering
    dc->SetTarget(bitmap.Get());
    // Draw something
    dc->BeginDraw();
    dc->Clear();
    ComPtr<ID2D1SolidColorBrush> brush;
    D2D1_COLOR_F const brushColor = D2D1::ColorF(0.18f, 0.55f, 0.34f, 0.75f); // rgba
    dc->CreateSolidColorBrush(brushColor, brush.GetAddressOf());
    D2D1_POINT_2F const ellipseCenter = D2D1::Point2F(150.0f, 150.0f); // y
    D2D1_ELLIPSE const ellipse = D2D1::Ellipse(ellipseCenter, 100.0f, 100.0f);
    dc->FillEllipse(ellipse, brush.Get());
    dc->EndDraw();
    // Make the swap chain available to the composition engine
    swapChain->Present(1, 0); //// sync flags



    DCompositionCreateDevice(dxgiDevice.Get(),__uuidof(dcompDevice),reinterpret_cast<void**>(dcompDevice.GetAddressOf()));
    dcompDevice->CreateTargetForHwnd(window,true, target.GetAddressOf()); // Top most        
    dcompDevice->CreateVisual(visual.GetAddressOf());
    visual->SetContent(swapChain.Get());
    target->SetRoot(visual.Get());
    dcompDevice->Commit();
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}