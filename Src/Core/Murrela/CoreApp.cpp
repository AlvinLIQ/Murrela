#ifndef UNICODE
#define UNICODE
#endif

#include "Headers/CoreApp.h"

using namespace Controls;

D2D1_POINT_2F curPos = {};
short param;

/* run murrela ui
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}
*/

void CoreApp::Run()
{
	ShowWindow(coreWindow, ParamNum);
	SetLayeredWindowAttributes(coreWindow, RGB(0, 0, 0), 0xD0, LWA_ALPHA);

	RECT scSize;
	GetClientRect(coreWindow, &scSize);
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

LRESULT CALLBACK CoreApp::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);

	case WM_KEYDOWN:
		Controls::_KeyReceived((unsigned int)wParam, false);
		break;
	case WM_CHAR:
		Controls::_CharacterReceived((unsigned int)wParam);
		Controls::_ReDrawRequest();
		break;
	case WM_MOUSEMOVE:
	{
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		_PointerRequest(&curPos, 1, content);
		Controls::_ReDrawRequest();
	}
	break;
	/*
	case WM_NCMOUSEMOVE:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		_PointerRequest(&curPos, 1, content);
		Controls::_ReDrawRequest();
		break;
		*/
	case WM_LBUTTONUP:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		content->PointerReleased(&curPos, 4);
		Controls::_ReDrawRequest();
		break;
	case WM_LBUTTONDOWN:
		curPos = D2D1::Point2F((FLOAT)LOWORD(lParam), (FLOAT)HIWORD(lParam));
		content->PointerPressed(&curPos, 4);
		Controls::_ReDrawRequest();
		break;
	case WM_SIZE:
	case WM_SIZING:
		UpdateSize();
		Controls::_ReDrawRequest();
		break;
	}
	UpdateWindow(hwnd);
//	SetLayeredWindowAttributes(coreWindow, 0x00FFFFFF, 0x80, LWA_COLORKEY);


End:
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