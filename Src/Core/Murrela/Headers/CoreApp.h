#ifndef UNICODE
#define UNICODE
#endif

#include "Controls.h"

class CoreApp
{
public:
	CoreApp(HINSTANCE hInstance);
	~CoreApp()
	{

	}

	Murrela* murrela = nullptr;
	Controls::Control* content = nullptr;

	void Run();
	void UpdateSize();
//	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	HWND coreWindow;
};