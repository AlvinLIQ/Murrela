#ifndef UNICODE
#define UNICODE
#endif

#include "Headers/CoreApp.h"
#include <dwmapi.h>

using namespace Controls;

D2D1_POINT_2F curPos = {};
short param;
CoreApp* coreApp = nullptr;
TRACKMOUSEEVENT tme;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* run murrela ui
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}
*/
CoreApp::CoreApp(HINSTANCE hInstance)
{
	WNDCLASS wc = {};
//	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Sample";

	RegisterClass(&wc);

	coreWindow = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	RECT scSize;
	GetClientRect(coreWindow, &scSize);
	murrela = new Murrela(coreWindow, D2D1::SizeF((FLOAT)(scSize.right - scSize.left), (FLOAT)(scSize.bottom - scSize.top)));

	coreApp = this;
}

void CoreApp::Run()
{
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = coreWindow;
	
	ShowWindow(coreWindow, SW_SHOWDEFAULT);
//	SetLayeredWindowAttributes(coreWindow, RGB(0, 0, 0), 0xD0, LWA_COLORKEY);

	//	content must be assigned a value
	//  content = (Control*)new Just_Editor::MainGrid(murrela);


	MSG msg = {};
	
	Controls::_StartReDrawLoop(&content);

	while (GetMessage(&msg, coreWindow, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (hwnd)
		{
			DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
			DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		Controls::_StopReDrawLoop();
		exit(1);
		return 0;
	case WM_KEYDOWN:
		Controls::_KeyReceived((unsigned int)wParam, false);
		break;

	case WM_IME_CHAR:
	case WM_CHAR:
		Controls::_CharacterReceived((unsigned int)wParam);
		Controls::_ReDrawRequest();
		break;
	case WM_MOUSEMOVE:
		TrackMouseEvent(&tme);
	case WM_MOUSELEAVE:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		_PointerRequest(&curPos, 1, coreApp->content);
		Controls::_ReDrawRequest();
	break;
	/*
	case WM_NCMOUSEMOVE:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		_PointerRequest(&curPos, 1, coreApp->content);
		Controls::_ReDrawRequest();
		break;
		*/
	case WM_LBUTTONUP:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		coreApp->content->PointerReleased(&curPos, 4);
		Controls::_ReDrawRequest();
		break;
	case WM_LBUTTONDOWN:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		coreApp->content->PointerPressed(&curPos, 4);
		Controls::_ReDrawRequest();
		break;
	case WM_MDIMAXIMIZE:
	case WM_SIZE:
	case WM_SIZING:
		coreApp->UpdateSize();
		Controls::_ReDrawRequest();
		break;
	}
	UpdateWindow(hwnd);
//	SetLayeredWindowAttributes(coreWindow, 0x00FFFFFF, 0x80, LWA_COLORKEY);

//End:
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CoreApp::UpdateSize()
{
	if (content != nullptr)
	{
		RECT scSize;
		GetClientRect(coreWindow, &scSize);
		D2D1_SIZE_F newSize = D2D1::SizeF((FLOAT)(scSize.right - scSize.left), (FLOAT)(scSize.bottom - scSize.top));
		murrela->SetSize(newSize);
		content->UpdateLayout();
	}
}