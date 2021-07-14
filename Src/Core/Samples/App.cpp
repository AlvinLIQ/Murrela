#include "../Murrela/Headers/CoreApp.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);
	coreApp->content = new Controls::Grid(coreApp->murrela, Controls::Stretch);
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}