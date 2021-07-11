#ifndef UNICODE
#define UNICODE
#endif

#include "Murrela/Headers/Controls.h"

class CoreApp
{
public:
	CoreApp(HINSTANCE hInstance)
	{
		WNDCLASS wc = {};
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = L"Sample";

		RegisterClass(&wc);

		coreWindow = CreateWindowEx(WS_EX_LAYERED, wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);

		murrela = new Murrela(coreWindow, D2D1::SizeF((FLOAT)(scSize.right - scSize.left), (FLOAT)(scSize.bottom - scSize.top)));
	}
	~CoreApp()
	{

	}

	Murrela* murrela = nullptr;
	Control* content = nullptr;

	void Run();
	void UpdateSize();
private:
	HWND coreWindow;
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};