#pragma once

#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite.h>
#include <dcomp.h>
#include <wincodec.h>

#ifndef _UWP
#include <vector>
#include <string>
#include <Windows.h>
#include <wrl/client.h>
#include <time.h>
#else
#include <Combaseapi.h>
#endif

static void SafeRelease(IUnknown** target)
{
	if (*target != nullptr)
	{
		(*target)->Release();
		*target = nullptr;
	}
}

static D2D1_SIZE_F RectToSize(D2D1_RECT_F tRect)
{
	return D2D1::SizeF(tRect.right - tRect.left, tRect.bottom - tRect.top);
}


class Murrela
{
public:
#ifdef _UWP
	Murrela(IUnknown* corewindow, D2D1_SIZE_F size = {});
#else
	Murrela(HWND corewindow, D2D1_SIZE_F size = {});
#endif
	~Murrela();

	void InitD2D();
	void InitResources();

	Microsoft::WRL::ComPtr<ID2D1Factory1> d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device> d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2dContext;
	Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> dxgiSwapChain;
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface;
#ifndef _UWP
	Microsoft::WRL::ComPtr<IDCompositionDevice> dCompDevice;
	Microsoft::WRL::ComPtr<IDCompositionTarget> dCompTarget;
#endif

	D2D1_RENDER_TARGET_PROPERTIES d2dProperties;

	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> defaultBackgroundBrush;

	Microsoft::WRL::ComPtr<IDWriteFactory> wrtFactory;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> txtFormat;
	Microsoft::WRL::ComPtr<IDWriteFontCollection> fonts;

	Microsoft::WRL::ComPtr<IWICImagingFactory2> wicFactory;

	DWRITE_TEXT_METRICS GetWCharWidth(const wchar_t tWChar, IDWriteTextLayout** idwrtLayout = nullptr)
	{
		DWRITE_TEXT_METRICS textMetrics;
		wrtFactory->CreateTextLayout(&tWChar, 1, txtFormat.Get(), 0, 0, idwrtLayout);
		(*idwrtLayout)->GetMetrics(&textMetrics);
		return textMetrics;
	}
	DWRITE_TEXT_METRICS GetWCStrWidth(const wchar_t* tWCStr, UINT32 len = -1, IDWriteTextLayout** idwrtLayout = nullptr)
	{
		DWRITE_TEXT_METRICS textMetrics;
#ifdef max
#undef max
#endif
		if (len == -1)
			len = (UINT32)wcslen(tWCStr);
		wrtFactory->CreateTextLayout(tWCStr, len, txtFormat.Get(), std::numeric_limits<float>::max(), 0, idwrtLayout);
		(*idwrtLayout)->GetMetrics(&textMetrics);

		return textMetrics;
	}

	static void GetRawSize(D2D1_SIZE_F* logicSize)
	{
#ifdef _UWP
		auto currentDisplayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
		logicSize->height = logicSize->height / 96 * currentDisplayInformation->LogicalDpi;
		logicSize->width = logicSize->width / 96 * currentDisplayInformation->LogicalDpi;
#endif
	}

	void SetSize(D2D1_SIZE_F newSize);
	void SetTargetBitmap();

	void LoadBitmapFromFilename(const wchar_t* filename, ID2D1Bitmap1** bitmap)
	{
		IWICBitmapDecoder* bitmapDecoder = NULL;
		wicFactory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder);

		IWICBitmapFrameDecode* bitmapFrame = NULL;
		bitmapDecoder->GetFrame(0, &bitmapFrame);
		SafeRelease((IUnknown**)&bitmapDecoder);

		IWICFormatConverter* bitmapConverter = NULL;
		wicFactory->CreateFormatConverter(&bitmapConverter);
		bitmapConverter->Initialize(bitmapFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);

		auto hr = d2dContext->CreateBitmapFromWicBitmap(bitmapConverter, NULL, bitmap);
		SafeRelease((IUnknown**)&bitmapConverter);
	}
	D2D1_SIZE_F d2dSize;
	float d2dDpiX, d2dDpiY;

	void DrawShadow(D2D1_RECT_F tRect, D2D1_POINT_2F tOffset = {})
	{
		ID2D1Effect* effect1;
		d2dContext->CreateEffect(CLSID_D2D1Shadow, &effect1);
		d2dContext->DrawImage(effect1);
	}

private:
#ifdef _UWP
	IUnknown* coreWindow;
#else
	HWND coreWindow;
#endif

};
