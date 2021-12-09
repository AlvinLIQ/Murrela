<h1>Murrela Docs</h1>

<h3>Get Started With First Murrela App</h3>

```cpp
#include <CoreApp.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);
	coreApp->content = new Controls::Grid(coreApp->murrela, Controls::Stretch)
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}
```