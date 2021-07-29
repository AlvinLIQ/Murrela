#include "../Murrela/Headers/CoreApp.h"

using namespace Controls;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPWSTR Param, int ParamNum)
{
	CoreApp* coreApp = new CoreApp(hInstance);

	//UI
	Grid* mGrid;
	coreApp->content = mGrid = new Grid(coreApp->murrela, Controls::Stretch);
	mGrid->AppendItem((Control*)new TextBlock(L"Sample Text", coreApp->murrela, Left | Top, Center));
	mGrid->AppendItem((Control*)new TextBox(coreApp->murrela, Right | Top, {180, 40}));
	Button* addTabBtn;
	mGrid->AppendItem((Control*)(addTabBtn = new Button(L"Add Tab", coreApp->murrela, Center, {100, 40})));

	TabContainer* tabContainer;
	mGrid->AppendItem((Control*)(tabContainer = new TabContainer(coreApp->murrela, Left | Bottom, {0, 40})));
	addTabBtn->ClickEvent.param = (void*)tabContainer;
	addTabBtn->Clicked.push_back([](void* param)
	{
			Tab* tabCon = (Tab*)param;
			tabCon->AppendItem(new Tab(L"Tab", tabCon->GetMurrela(), Left | Stretch));
	});



	coreApp->Run();
	coreApp->~CoreApp();
	return 0;
}