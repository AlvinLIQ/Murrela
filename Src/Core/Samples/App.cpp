#include "../Murrela/Headers/CoreApp.h"
#include "../Murrela/Headers/Tools.h"

using namespace Controls;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);

	//UI
//	ScrollViewer* mGrid;
//	coreApp->content = (Control*)(mGrid = new ScrollViewer(coreApp->murrela, Controls::Stretch));
	TextBox* tb;
	coreApp->content = (Control*)(tb = new TextBox(coreApp->murrela, Stretch));
//	mGrid->SetContent((Control*)(tb = new TextBox(coreApp->murrela, Stretch)));
//	const char* result;
//	ReadFileFromPath(L"D:/Dev/PHPServer/index.php", (void**)&result);
//	tb->InsertTextAt(ctowc(result));
//	mGrid->AppendItem((Control*)new TextBox(coreApp->murrela, Stretch, {}));
//	tabContainer->AppendItem(new Tab(L"Tab", tabContainer->GetMurrela(), Left | Stretch));
	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}
