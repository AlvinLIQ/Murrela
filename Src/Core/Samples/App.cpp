#include "../Murrela/Headers/CoreApp.h"

using namespace Controls;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);

	//UI
	ScrollViewer* mGrid;
	coreApp->content = (Control*)(mGrid = new ScrollViewer(coreApp->murrela, Controls::Stretch));
	mGrid->SetContent((Control*)new TextBox(coreApp->murrela, Stretch, {}));
//	mGrid->AppendItem((Control*)new TextBox(coreApp->murrela, Stretch, {}));
//	tabContainer->AppendItem(new Tab(L"Tab", tabContainer->GetMurrela(), Left | Stretch));
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}
