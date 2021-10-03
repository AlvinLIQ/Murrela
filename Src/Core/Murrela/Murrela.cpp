#include "Headers/Murrela.h"

using namespace Microsoft::WRL;

#ifdef _UWP
Murrela::Murrela(IUnknown* corewindow, D2D1_SIZE_F size)
#else
Murrela::Murrela(HWND corewindow, D2D1_SIZE_F size)
#endif
{
    coreWindow = corewindow;
    d2dSize = size;
	InitD2D();
}

Murrela::~Murrela()
{
	d2dFactory.Reset();
	d2dDevice.Reset();
    dxgiSurface.Reset();
}

void Murrela::InitD2D()
{
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
    D3D_FEATURE_LEVEL m_featureLevels;
    D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE,
        0, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, d3dDevice.GetAddressOf(),
        &m_featureLevels, d3dContext.GetAddressOf());

    d3dDevice.As(&dxgiDevice);
    dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
    dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

    D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, d2dFactory.GetAddressOf());
    d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf());
    d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dContext.GetAddressOf());

#ifndef _UWP
    DCompositionCreateDevice(dxgiDevice.Get(),
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(dCompDevice.GetAddressOf()));
#endif

    SetSize(d2dSize);

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(wrtFactory), reinterpret_cast<IUnknown**>(wrtFactory.GetAddressOf()));
    
    InitResources();
}

void Murrela::InitResources()
{
    d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &defaultBackgroundBrush);

    wrtFactory->GetSystemFontCollection(&fonts);
    wrtFactory->CreateTextFormat(L"Source Code Variable", fonts.Get(), DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18, L"", &txtFormat);
}

void Murrela::SetTargetBitmap()
{
    dxgiSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));

    D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        D2D1_ALPHA_MODE_PREMULTIPLIED
    );

    float dpi;
#ifdef _UWP
    auto currentDisplayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    dpi = currentDisplayInformation->LogicalDpi;
#else
    dpi = 96.0f;
#endif
    
    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        pixelFormat, dpi, dpi);

    ComPtr<ID2D1Bitmap1> d2dBitmap;
    
    d2dContext->SetDpi(dpi, dpi);
    d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dBitmap);
    d2dContext->SetTarget(d2dBitmap.Get());
    d2dContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
}

void Murrela::SetSize(D2D1_SIZE_F newSize)
{
    d2dSize = newSize;
    GetRawSize(&newSize);
    if (dxgiSwapChain == nullptr)
    {
        swapChainDesc = { 0 };
        swapChainDesc.Width = (UINT)newSize.width;                           // use automatic sizing
        swapChainDesc.Height = (UINT)newSize.height;
#ifdef _UWP
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FOREGROUND_LAYER;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        dxgiFactory->CreateSwapChainForCoreWindow(dxgiDevice.Get(), coreWindow, &swapChainDesc, nullptr, dxgiSwapChain.GetAddressOf());
#else
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
//        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        
//        dxgiFactory->CreateSwapChainForHwnd(dxgiDevice.Get(), coreWindow, &swapChainDesc, nullptr, nullptr, dxgiSwapChain.GetAddressOf());
        dxgiFactory->CreateSwapChainForComposition(dxgiDevice.Get(), &swapChainDesc, NULL, &dxgiSwapChain);
        
        dCompDevice->CreateTargetForHwnd(coreWindow, TRUE, dCompTarget.GetAddressOf());
        IDCompositionVisual* rVisual;
        dCompDevice->CreateVisual(&rVisual);
        rVisual->SetContent(dxgiSwapChain.Get());
        dCompTarget->SetRoot(rVisual);
        dCompDevice->Commit();
#endif
    }
    else
    {
        d2dContext->SetTarget(nullptr);
        dxgiSurface.ReleaseAndGetAddressOf();
#ifdef _UWP
        dxgiSwapChain->ResizeBuffers(2, (UINT)newSize.width, (UINT)newSize.height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FOREGROUND_LAYER);
#else
        dxgiSwapChain->ResizeBuffers(2, (UINT)newSize.width, (UINT)newSize.height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE);
#endif
    }
    SetTargetBitmap();
}