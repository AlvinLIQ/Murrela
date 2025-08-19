#include "Headers/Murrela.h"

using namespace Microsoft::WRL;
using namespace DirectX;

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
    dxgiOutput.Reset();
    dxgiSurface.Reset();
    dxgiAdapter.Reset();
	d2dFactory.Reset();
	d2dDevice.Reset();
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
    dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    dxgiOutput.As(&dxgiOutput1);

    D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, d2dFactory.GetAddressOf());
    d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf());
    d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dContext.GetAddressOf());

#ifndef _UWP
    DCompositionCreateDevice(dxgiDevice.Get(),
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(dCompDevice.GetAddressOf()));
    ArrowCursor = LoadCursor(NULL, IDC_ARROW);
    IbeamCursor = LoadCursor(NULL, IDC_IBEAM);
    CurrentCursor = ArrowCursor;
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

bool Murrela::SaveTextureAsPNG(ID3D11Device* device,
    ID3D11DeviceContext* context,
    ID3D11Texture2D* texture,
    const wchar_t* filename)
{
    if (!device || !context || !texture || !filename) return false;

    // 1. Get texture description
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    // 2. Create staging texture (CPU-readable)
    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> staging;
    if (FAILED(device->CreateTexture2D(&stagingDesc, nullptr, &staging)))
        return false;

    context->CopyResource(staging.Get(), texture);

    // 3. Map the staging texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped)))
        return false;


    // 5. Create a WIC bitmap
    ComPtr<IWICBitmap> wicBitmap;
    if (FAILED(wicFactory->CreateBitmapFromMemory(
        desc.Width, desc.Height,
        GUID_WICPixelFormat32bppBGRA,
        mapped.RowPitch,
        mapped.RowPitch * desc.Height,
        (BYTE*)mapped.pData,
        &wicBitmap)))
    {
        context->Unmap(staging.Get(), 0);
        return false;
    }

    context->Unmap(staging.Get(), 0);

    // 6. Create PNG encoder
    ComPtr<IWICStream> stream;
    if (FAILED(wicFactory->CreateStream(&stream))) return false;
    if (FAILED(stream->InitializeFromFilename(filename, GENERIC_WRITE))) return false;

    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(wicFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder))) return false;
    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) return false;

    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    if (FAILED(encoder->CreateNewFrame(&frame, &props))) return false;
    if (FAILED(frame->Initialize(props.Get()))) return false;
    if (FAILED(frame->SetSize(desc.Width, desc.Height))) return false;

    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(frame->SetPixelFormat(&format))) return false;
    if (FAILED(frame->WriteSource(wicBitmap.Get(), nullptr))) return false;
    if (FAILED(frame->Commit())) return false;
    if (FAILED(encoder->Commit())) return false;

    return true;
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
    /*
    ID3D11Buffer* g_pVertexBuffer;

    // Define the data-type that
    // describes a vertex.
    struct SimpleVertexCombined
    {
        XMFLOAT3 Pos;
        XMFLOAT3 Col;
    };

    // Supply the actual vertex data.
    SimpleVertexCombined verticesCombo[] =
    {
        XMFLOAT3(0.0f, 0.5f, 0.5f),
        XMFLOAT3(0.0f, 0.0f, 0.5f),
        XMFLOAT3(0.5f, -0.5f, 0.5f),
        XMFLOAT3(0.5f, 0.0f, 0.0f),
        XMFLOAT3(-0.5f, -0.5f, 0.5f),
        XMFLOAT3(0.0f, 0.5f, 0.0f),
    };

    // Fill in a buffer description.
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(SimpleVertexCombined) * 3;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    // Fill in the subresource data.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = verticesCombo;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;

    // Create the vertex buffer.
    d3dDevice->CreateBuffer(&bufferDesc, &InitData, &g_pVertexBuffer);

    UINT stride = sizeof(XMFLOAT3) * 6;
    UINT offset = 0;
    d3dContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    */
    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        pixelFormat, dpi, dpi);

    d2dContext->SetDpi(dpi, dpi);
    d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dBitmap);
    d2dContext->SetTarget(d2dBitmap.Get());
    d2dContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
}

void Murrela::SetSize(D2D1_SIZE_F newSize)
{
    std::scoped_lock lock(swapChainMutex);
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
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;                     // use double buffering to enable flip
        swapChainDesc.Scaling = DXGI_SCALING_NONE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
        swapChainDesc.Flags = 0;

        dxgiFactory->CreateSwapChainForHwnd(dxgiDevice.Get(), coreWindow, &swapChainDesc, nullptr, nullptr, dxgiSwapChain.GetAddressOf());


        /*
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        swapChainDesc.Scaling = DXGI_SCALING_NONE;

        dxgiFactory->CreateSwapChainForComposition(d3dDevice.Get(), &swapChainDesc, NULL, &dxgiSwapChain);
        
        dCompDevice->CreateTargetForHwnd(coreWindow, TRUE, dCompTarget.GetAddressOf());
        IDCompositionVisual* rVisual;
        dCompDevice->CreateVisual(&rVisual);
        rVisual->SetContent(dxgiSwapChain.Get());
        dCompTarget->SetRoot(rVisual);
        dCompDevice->Commit();*/
#endif
    }
    else
    {
        d2dContext->SetTarget(nullptr);
        d2dBitmap.Reset();
        dxgiSurface.Reset();
#ifdef _UWP
        dxgiSwapChain->ResizeBuffers(2, (UINT)newSize.width, (UINT)newSize.height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FOREGROUND_LAYER);
#else
        dxgiSwapChain->ResizeBuffers(2, (UINT)newSize.width, (UINT)newSize.height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE);
#endif
    }
    SetTargetBitmap();
}