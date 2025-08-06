#ifndef UNICODE
#define UNICODE
#endif

#include "Controls.h"

class CoreApp
{
public:
	CoreApp(HINSTANCE hInstance, LPCWSTR className = L"Murrela");
	~CoreApp()
	{

	}

	Murrela* murrela = nullptr;
	Controls::Control* content = nullptr;

	std::vector<void(*)(void* param)> SizeChanged;
	Controls::Control::NormalEvent ResizeEvent;
	void CallEvent(Controls::Control::NormalEvent* tEvent)
	{
		auto events = (std::vector<void(*)(void* param)>*)tEvent->events;
		for (auto i = events->begin(); i != events->end(); i++)
		{
			(*i)(tEvent->param);
		}
	}

	void Run();
	void UpdateSize();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND GetHandler()
	{
		return coreWindow;
	}
private:
	HWND coreWindow;

};