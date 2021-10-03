#include "Headers/Controls.h"

using namespace Controls;

Control* focusedControl = nullptr;
clock_t passed, delay = 200;
clock_t lastCount, currentCount;
std::vector<void(*)(void* param)>* ticks = nullptr;
void* tickParam;
bool tickAbort = false;
//0 close 1 redraw 2 waiting 
short drawSignal = 1;


Control::Control(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlRect)
{
	murrela = murla;
	Alignment = alignment;

	ControlSize = controlRect;
	float maxFloat = std::numeric_limits<float>::max();
	maxWidth = maxFloat;
	maxHeight = maxFloat;
	minWidth = ControlSize.width;
	minHeight = ControlSize.height;
}

void Control::Focus()
{
	if (focusedControl != nullptr && focusedControl != this)
		focusedControl->CancelFocus();
	focusedControl = this;
}

void Control::CancelFocus()
{
	if (focusedControl == this)
		focusedControl = nullptr;
}

void Control::ReDraw()
{
	murrela->d2dContext->BeginDraw();
	if (this->Parent == nullptr)
#ifndef _UWP
		murrela->d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White));
#else
		murrela->d2dContext->Clear(WhiteColor);
#endif
	else
		murrela->d2dContext->FillRectangle(GetRectForRender(), murrela->defaultBackgroundBrush.Get());
	Draw();
	murrela->d2dContext->EndDraw();
	murrela->dxgiSwapChain->Present(0, 0);
}

bool Controls::_IsControlFocused(Control* tControl)
{
	return focusedControl == tControl && tControl != nullptr;
}

void Controls::_PointerRequest(D2D1_POINT_2F* pPosition, short pState, Control* target)
{
	if (focusedControl != nullptr && pState)
		focusedControl->PointerMoved(pPosition, pState);
	else
		target->PointerMoved(pPosition, pState);

}

void Controls::_CharacterReceived(unsigned int keycode)
{
	if (focusedControl != nullptr)
		focusedControl->CharacterReceived(keycode);
}

void Controls::_KeyReceived(unsigned int keycode, bool isReleased)
{
	if (focusedControl != nullptr)
		focusedControl->KeyReceived(keycode, isReleased);
}

void Controls::_ReDrawLoop(Control** content)
{
	while (drawSignal)
	{
		if (drawSignal & 1)
		{
			(*content)->ReDraw();
			drawSignal = 2;
		}
	}
}

void Controls::_StartReDrawLoop(Control** content)
{
	HANDLE thHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_ReDrawLoop, (LPVOID)content, 0, 0);
}

void Controls::_StopReDrawLoop()
{
	drawSignal = 0;
}

void Controls::_ReDrawRequest()
{
	drawSignal = 1;
}

void ItemsContainer::PointerMoved(D2D1_POINT_2F* pPosition, short pState)
{
	size_t size = GetItemsNum();
	if (size)
	{
		size_t i;
		for (i = 0; i < size; i++)
		{
			if (items[i]->IsInside(pPosition))
			{
				items[i]->PointerMoved(pPosition, pState);
				break;
			}
		}
		if (pointerIndex < size && items[pointerIndex]->GetPointerState() ^ 3 && pointerIndex != i)
			items[pointerIndex]->PointerExited(pPosition, pState);
		pointerIndex = i;
	}
	Control::PointerMoved(pPosition, pState);
}

void ItemsContainer::PointerReleased(D2D1_POINT_2F* pPosition, short pState)
{
	if (focusedControl != nullptr && focusedControl != this)
		focusedControl->PointerReleased(pPosition, pState);

	Control::PointerReleased(pPosition, pState);
}

bool Tab::IsSelected()
{
	return ((TabContainer*)this->Parent)->SelectedTab == this;
}

void Tick()
{
	passed = 0L;
	lastCount = clock();
	size_t s = ticks->size(), i;
	while (!tickAbort)
	{
		currentCount = clock();
		passed += currentCount - lastCount;
		if (passed > 50)
		{
			for (i = 0; i < s; i++)
				(*ticks)[i](tickParam);
			passed = 0L;
			//Redraw
			drawSignal = 1;
		}
		lastCount = currentCount;
	}
	tickParam = nullptr;
	ticks = nullptr;
}

void RepeatButton::PointerPressed(D2D1_POINT_2F* pPosition, short pState)
{
	Container::PointerPressed(pPosition, pState);
	//start clicking
	tickParam = ClickEvent.param;
	ticks = (std::vector<void(*)(void* param)>*)ClickEvent.events;
	tickAbort = false;

	CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Tick, NULL, 0, NULL));
}

void RepeatButton::PointerReleased(D2D1_POINT_2F* pPosition, short pState)
{
	//stop clicking
	Container::PointerReleased(pPosition, pState);
	tickAbort = true;
}

