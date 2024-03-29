﻿#pragma once
#include "Murrela.h"

#define AutoSize (D2D1_SIZE_F){0.0f, 0.0f}
#define DefaultAlignment Left | Top

#define WhiteColor D2D1::ColorF(D2D1::ColorF::White)


#ifdef _UWP
static bool isKeyDown(Windows::System::VirtualKey vKey)
{
	return Windows::UI::Core::CoreWindow::GetForCurrentThread()->GetAsyncKeyState(vKey) == Windows::UI::Core::CoreVirtualKeyStates::Down;
#else
static bool isKeyDown(int vKey)
{
	return GetAsyncKeyState(vKey);
#endif
}

namespace Controls
{
	void _ReDrawRequest();

#define HAS_PARENT (Parent != nullptr)
	typedef char Alignments;
	const Alignments Left = 1, Top = 2, Right = 4, Bottom = 8, Center = 16, Stretch = 32;
	class Control
	{
	public:
		Control(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {});

		std::wstring Text;

		D2D1_POINT_2F ControlOffset;
		D2D1_SIZE_F ControlSize;

		Alignments Alignment;

		bool IsVisible = true;
		// 0 keep focus until another one focused 1 cancel focus when pointer released
		short FocusType = 1;

		typedef struct
		{
			void* events;
			void* param;
		} NormalEvent;

		virtual void KeyReceived(unsigned int keyCode, bool isReleased)
		{

		}
		virtual void CharacterReceived(unsigned int keyCode)
		{

		}

		virtual void Draw() = 0;
		virtual void ReDraw();

		virtual void Focus();

		virtual void CancelFocus();

		virtual short GetPointerState()
		{
			return pointerState;
		}
		virtual void GetMaxSizeForChild(D2D1_SIZE_F* maxSize, Control* child = nullptr)
		{
			if (child == nullptr)
				*maxSize = ControlSize;
			else
				*maxSize = D2D1::SizeF(child->ControlSize.width - (child->ControlOffset.x - ControlOffset.x), child->ControlSize.height - (child->ControlOffset.y - ControlOffset.y));
		}

		Murrela* GetMurrela()
		{
			return murrela;
		}

		bool IsInside(D2D1_POINT_2F* pPosition)
		{
			auto ControlRect = GetRectForRender();
			return ControlOffset.x <= pPosition->x && ControlRect.right >= pPosition->x && ControlOffset.y <= pPosition->y && ControlRect.bottom >= pPosition->y;
		}

		virtual void PointerEntered(D2D1_POINT_2F* pPosition, short pState)
		{
			pointerState = 1;
		}
		virtual void PointerExited(D2D1_POINT_2F* pPosition, short pState)
		{
			pointerState = 0;
		}
		virtual void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
			pointerState = 2;
			Focus();
		}
		virtual void PointerReleased(D2D1_POINT_2F* pPosition, short pState)
		{
			pointerState = IsInside(pPosition) ? 1 : 0;
			if (FocusType & 1)
				CancelFocus();
		}
		virtual void PointerMoved(D2D1_POINT_2F* pPosition, short pState)
		{
			if (pointerState != 1 && pointerState != 2 && IsInside(pPosition))
				PointerEntered(pPosition, pState);

			for (auto i = PointerMovedEvents.begin(); i < PointerMovedEvents.end(); i++)
				(*i)(pPosition, pState);
		}
		std::vector<void(*)(D2D1_POINT_2F * pPosition, short pState)> PointerMovedEvents;

		virtual bool SizeRequest(D2D1_SIZE_F newSize)
		{
			/*
			if (HAS_PARENT)
			{
				D2D1_SIZE_F visibleSize;
				Parent->GetMaxSizeForChild(&visibleSize, this);
				if (visibleSize.width <= 0 || visibleSize.height <= 0)
					return false;
			}
			*/
			D2D1_SIZE_F maxSize = { maxWidth, maxHeight }, minSize = { minWidth, minHeight };
			Alignments tAlign = Alignment ^ Stretch;
			if (tAlign ^ Center)
			{
				if (HAS_PARENT)
				{
					if (Parent->maxWidth < maxWidth)
						maxSize.width = Parent->maxWidth;
					if (Parent->maxHeight < maxHeight)
						maxSize.height = Parent->maxHeight;
				}
			}
			if (newSize.width > maxSize.width)
				newSize.width = maxSize.width;
			else if (newSize.width < minSize.width)
				newSize.width = minSize.width;
			if (newSize.height > maxSize.height)
				newSize.height = maxSize.height;
			else if (newSize.height < minSize.height)
				newSize.height = minSize.height;

			ControlSize = newSize;
			if (HAS_PARENT)
			{
				if ((newSize.width = ControlSize.width + ControlOffset.x) > Parent->ControlSize.width + Parent->ControlOffset.x || Parent->maxWidthChild == this)
				{
					Parent->ControlSize.width = newSize.width - Parent->ControlOffset.x;
					Parent->maxWidthChild = this;
				}
				if ((newSize.height = ControlSize.height + ControlOffset.y) > Parent->ControlSize.height + Parent->ControlOffset.y || Parent->maxHeightChild == this)
				{
					Parent->ControlSize.height = newSize.height - Parent->ControlOffset.y;
					Parent->maxHeightChild = this;
				}
				Parent->UpdateLayout();
			}
			SizeChanged(ControlSize);

			return true;
		}
		virtual void UpdateLayout()
		{
			D2D1_RECT_F pRect = GetParentRect();
			D2D1_SIZE_F pSize = GetParentSize(), prevSize = ControlSize;

			if (Alignment & Left)
			{
				ControlOffset.x = pRect.left;
			}
			else if (Alignment & Right)
			{
				ControlOffset.x = pRect.right - ControlSize.width;
			}
			else if (Alignment & Center)
			{
				ControlOffset.x = pRect.left + (pSize.width - ControlSize.width) / 2;
			}
			else
			{
				ControlOffset.x = pRect.left;
				ControlSize.width = pRect.right - pRect.left;
			}
			if (Alignment & Top)
			{
				ControlOffset.y = pRect.top;
			}
			else if (Alignment & Bottom)
			{
				ControlOffset.y = pRect.bottom - ControlSize.height;
			}
			else if (Alignment & Center)
			{
				ControlOffset.y = pRect.top + (pSize.height - ControlSize.height) / 2;
			}
			else
			{
				ControlOffset.y = pRect.top;
				ControlSize.height = pRect.bottom - pRect.top;
			}

			if (prevSize.height != ControlSize.height || prevSize.width != ControlSize.width)
			{
				SizeChanged(ControlSize);
			}
		}
		virtual void SizeChanged(D2D1_SIZE_F newSize)
		{
			
		}
		virtual D2D1_RECT_F GetRectForRender()
		{
			return { ControlOffset.x, ControlOffset.y, ControlOffset.x + ControlSize.width, ControlOffset.y + ControlSize.height };
		}

		virtual D2D1_SIZE_F GetRealSize()
		{
			return ControlSize;
		}

		void SetParent(Control* nParent)
		{
			Parent = nParent;
		}

		Control* Parent = nullptr;
	protected:
		D2D1_SIZE_F GetParentSize()
		{
			return Parent == nullptr ? murrela->d2dSize : Parent->ControlSize;
		}
		D2D1_RECT_F GetParentRect()
		{
			return Parent == nullptr ? D2D1::RectF(0.0f, 0.0f, murrela->d2dSize.width, murrela->d2dSize.height) : Parent->GetRectForRender();
		}
		D2D1_POINT_2F GetPositionForCurrent(D2D1_POINT_2F pos)
		{
			pos.x -= ControlOffset.x;
			pos.y -= ControlOffset.y;
			return pos;
		}

		void CallEvent(NormalEvent* tEvent)
		{
			auto events = (std::vector<void(*)(void* param)>*)tEvent->events;
			for (auto i = events->begin(); i != events->end(); i++)
			{
				(*i)(tEvent->param);
			}
		}

		Murrela* murrela;
		Control* maxWidthChild = nullptr;
		Control* maxHeightChild = nullptr;
		short pointerState = 0;
		ID2D1SolidColorBrush* brushes[4];
		const UINT colors[4] = { 0x66DFDFDF, 0x66A0A0A0, 0x66444444, 0x66999999 };
		const int colCount = 4;
		virtual void Init()
		{
			for (int i = 0; i < colCount; i++)
			{
				//rgba
				murrela->d2dContext->CreateSolidColorBrush(D2D1::ColorF(
					(float)((colors[i] >> 0x10) & 0xFF) / 0xFF,
					(float)((colors[i] >> 0x8) & 0xFF) / 0xFF,
					(float)(colors[i] & 0xFF) / 0xFF,
					(float)(colors[i] >> 0x18) / 0xFF), &brushes[i]);
			}
		}
		float minWidth, minHeight, maxWidth, maxHeight;
	};
	void _PointerRequest(D2D1_POINT_2F* pPosition, short pState, Control* target);

	class TextBlock : Control
	{
	public:
		TextBlock(const wchar_t* txt, Murrela* murla, Alignments alignment, Alignments textAlignment, D2D1_SIZE_F blockSize = {}) :Control(murla, alignment, blockSize)
		{
			TextAlignment = textAlignment;
			SetText(txt);
			Init();
		}

		size_t Length;
		void SetText(const wchar_t* newText)
		{
			text = newText;
			Length = wcslen(newText);
			SafeRelease((IUnknown**)&textLayout);
			TextMetrics = murrela->GetWCStrWidth(newText, Length, &textLayout);
			updateTextMetrics();
		}
		void SetFontSize(FLOAT fontSize)
		{
			if (textLayout != nullptr)
			{
				textLayout->SetFontSize(fontSize, { (UINT32)0, (UINT32)text.length() });
				textLayout->GetMetrics(&TextMetrics);
				updateTextMetrics();
			}
		}
		const wchar_t* GetText()
		{
			return text.c_str();
		}

		Alignments TextAlignment;
		DWRITE_TEXT_METRICS TextMetrics;

		void Draw()
		{
			murrela->d2dContext->DrawTextLayout(D2D1::Point2F(ControlOffset.x + offsets.x, ControlOffset.y + offsets.y), textLayout, brush);
//			murrela->DrawShadow(GetRectForRender(), { 10.0f, 10.0f });
		}


	private:
		std::wstring text;
		ID2D1SolidColorBrush* brush;
		IDWriteTextLayout* textLayout = nullptr;

		D2D1_POINT_2F offsets;
		void updateTextMetrics()
		{
			if (TextMetrics.widthIncludingTrailingWhitespace < maxWidth)
				ControlSize.width = TextMetrics.widthIncludingTrailingWhitespace > minWidth ? TextMetrics.widthIncludingTrailingWhitespace : minWidth;
			else
				ControlSize.width = maxWidth;
			if (TextMetrics.height > ControlSize.height)
				ControlSize.height = TextMetrics.height;

			if (TextAlignment & Left)
				offsets.x = 0;
			else if (TextAlignment & Right)
				offsets.x = ControlSize.width - TextMetrics.widthIncludingTrailingWhitespace;
			else if (TextAlignment & Center)
				offsets.x = (ControlSize.width - TextMetrics.widthIncludingTrailingWhitespace) / 2;

			if (TextAlignment & Top)
				offsets.y = 0;
			else if (TextAlignment & Bottom)
				offsets.y = ControlSize.height - TextMetrics.height;
			else if (TextAlignment & Center)
				offsets.y = (ControlSize.height - TextMetrics.height) / 2;
		}
	protected:
		void Init()
		{
			murrela->d2dContext->CreateSolidColorBrush({ 0.0f, 1.0f, 1.0f , 1.0f }, &brush);
		}
	};

	class Image : Control
	{
	public:
		Image(Murrela* murla, Alignments alignment, D2D1_SIZE_F blockSize = {}) :Control(murla, alignment, blockSize)
		{
		}
		~Image()
		{
			SafeRelease((IUnknown**)&bitmap);
		}

		void Draw()
		{
			if (bitmap != nullptr)
				murrela->d2dContext->DrawBitmap(bitmap, GetRectForRender());
		}

		void SetBitmap(ID2D1Bitmap1* newBitmap)
		{
			if (bitmap != nullptr)
				SafeRelease((IUnknown**)&bitmap);
			bitmap = newBitmap;
		}
		void SetBitmapFromFile(const wchar_t* filename)
		{
			if (bitmap != nullptr)
				SafeRelease((IUnknown**)&bitmap);
			murrela->LoadBitmapFromFilename(filename, &bitmap);
		}

	protected:
		ID2D1Bitmap1* bitmap = nullptr;
	};

	class TextBox : Control
	{
	public:
		TextBox(Murrela* murla, Alignments alignment, D2D1_SIZE_F blockSize = {}) :Control(murla, alignment, blockSize)
		{
			murrela->d2dContext->CreateLayer(&d2dLayer);

			murrela->d2dContext->CreateSolidColorBrush(D2D1::ColorF(0xC0C0C0, 0.5), &brushes[0]);
			murrela->d2dContext->CreateSolidColorBrush(D2D1::ColorF(0xE0E0E0, 0.5), &brushes[1]);
			murrela->d2dContext->CreateSolidColorBrush(D2D1::ColorF(0xF0F0F0, 0.5), &brushes[2]);
			murrela->d2dContext->CreateSolidColorBrush(D2D1::ColorF(0x222222, 0.5), &brushes[3]);

			FocusType = 0;
		}
		~TextBox()
		{
			SafeRelease((IUnknown**)&d2dLayer);
		}

		void KeyReceived(unsigned int keyCode, bool isReleased)
		{
#ifdef _UWP
			bool isCtrlHeld = isKeyDown(Windows::System::VirtualKey::Control), isShiftHeld = isKeyDown(Windows::System::VirtualKey::Shift);
#else
			bool isCtrlHeld = isKeyDown(VK_CONTROL), isShiftHeld = isKeyDown(VK_SHIFT);
#endif
			switch (keyCode)
			{
			case VK_LEFT:
				if (isShiftHeld && cursor)
				{
					MoveTo(cursor - 1);
					if (selectionMetrics.size())
					{
						const auto& tMetrics = selectionMetrics.back();
						if (cursor < tMetrics.textPosition)
							Select(tMetrics.textPosition + tMetrics.length);
						else
							Select(tMetrics.textPosition);
					}
					else
						Select(cursor + 1);
				}
				else if (!isShiftHeld && selectionMetrics.size())
				{
					UINT32 pos = selectionMetrics.front().textPosition;
					MoveTo(min(pos, cursor));
					CancelSelection();
				}
				else if (cursor)
					MoveTo(cursor - 1);
				break;
			case VK_RIGHT:
				if (isShiftHeld && cursor < length)
				{
					MoveTo(cursor + 1);
					if (selectionMetrics.size())
					{
						const auto& tMetrics = selectionMetrics.front();
						if (cursor > tMetrics.textPosition + tMetrics.length)
							Select(tMetrics.textPosition);
						else
							Select(tMetrics.textPosition + tMetrics.length);
					}
					else
						Select(cursor - 1);
				}
				else if (!isShiftHeld && selectionMetrics.size())
				{
					const auto& tMetrics = selectionMetrics.back();
					UINT32 pos = tMetrics.textPosition + tMetrics.length;
					MoveTo(pos > cursor ? pos : cursor);
					CancelSelection();
				}
				else if (cursor < length)
					MoveTo(cursor + 1);
				break;
			case VK_DELETE:
				if (!ClearSelection() && cursor < length)
				{
					text = text.substr(0, cursor) + text.substr((size_t)cursor + 1, --length - cursor);
					UpdateText();
					break;
				}
			default:
				return;
			}
			Controls::_ReDrawRequest();
		}
		void CharacterReceived(unsigned int keyCode)
		{
			switch (keyCode)
			{
			case 8:
			{
				if (!ClearSelection() && cursor && length)
				{
					text = text.substr(0, cursor - 1) + text.substr(cursor, length - cursor);
					cursor--;
					length--;
					break;
				}
				Controls::_ReDrawRequest();
				return;
			}
			case 1:
				cursor = 0;
				Select(length);
				break;
			case 3:
			{
				/*
				if (!OpenClipboard(murrela->GetWindow()))
					return;

				auto pcstrS = wctoc(GetSelectionText().c_str());
				auto pcstrLen = strnlen(pcstrS, -1);
				auto hlgb = GlobalAlloc(GMEM_MOVEABLE, pcstrLen + 1);
				auto pcstrC = GlobalLock(hlgb);
				memcpy(pcstrC, pcstrS,pcstrLen);
				GlobalUnlock(hlgb);
				SetClipboardData(CF_TEXT, hlgb);
				//InsertTextAt()
				CloseClipboard();
				*/

				CopyWCStr(GetSelectionText().c_str());
				Controls::_ReDrawRequest();
				break;
			}
			case 22:
			{
				if (!IsClipboardFormatAvailable(CF_TEXT) || !OpenClipboard(murrela->GetWindow()))
					return;
				auto hlgb = GetClipboardData(CF_TEXT);
				if (hlgb != NULL)
				{
					auto lpstr = GlobalLock(hlgb);
					if (lpstr != NULL)
					{
						size_t prevLen = length;
						InsertTextAt(ctowc((const char*)lpstr), cursor);
						cursor += length - prevLen;
						GlobalUnlock(lpstr);
					}
				}
				//InsertTextAt()
				CloseClipboard();
				Controls::_ReDrawRequest();
			}
				break;
			case 24:
				CopyWCStr(GetSelectionText().c_str());

				ClearSelection();
				Controls::_ReDrawRequest();
				break;
			default:
				ClearSelection();
				if (keyCode == '\r')
					keyCode = '\n';
				if (cursor == length)
					text += (wchar_t)keyCode;
				else
					text = text.substr(0, cursor) + (wchar_t)keyCode + text.substr(cursor, length);
				cursor++;
				length++;
			}
			UpdateText();
			UpdateCursor();

			Controls::_ReDrawRequest();
		}

		void Draw()
		{
			D2D1_RECT_F tRect = GetRectForRender();
			murrela->d2dContext->PushLayer(D2D1::LayerParameters1(tRect), d2dLayer);

			murrela->d2dContext->FillRectangle(tRect, brushes[brushIndex]);
			float topMargin = ControlOffset.y + 5;
			size_t _size = selectionMetrics.size();
			for (size_t i = 0; i < _size; i++)
			{
				DWRITE_HIT_TEST_METRICS& tMetrics = selectionMetrics[i];
				tMetrics.top -= offsetY;
				tMetrics.left -= offsetX;
				murrela->d2dContext->FillRectangle(D2D1::RectF(tMetrics.left, tMetrics.top, tMetrics.left + tMetrics.width, tMetrics.top + tMetrics.height), brushes[1]);
			}
			if (drawingTextLayout != nullptr)
			{
				while (isTextLayoutLocked);
				murrela->d2dContext->DrawTextLayout(D2D1::Point2F(ControlOffset.x + 2, topMargin), drawingTextLayout, brushes[3], D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_NONE);
			}
			if (brushIndex == 2)
			{
				murrela->d2dContext->DrawLine(D2D1::Point2F(cursorX + ControlOffset.x + 2, topMargin + cursorY), D2D1::Point2F(cursorX + ControlOffset.x + 2, topMargin + cursorY + textHeight), brushes[0]);
			}

			murrela->d2dContext->PopLayer();
		}

		void CancelFocus()
		{
			brushIndex = 0;
		}

		const wchar_t* GetText()
		{
			return text.c_str();
		}

		const std::wstring GetSelectionText()
		{
			size_t _size = selectionMetrics.size();
			std::wstring sText = L"";
			if (_size)
			{
				size_t _offset = 0, _off;
				for (size_t i = 0; i < _size; i++)
				{
					DWRITE_HIT_TEST_METRICS& tMetrics = selectionMetrics[i];
					_off = tMetrics.textPosition - _offset;
					sText += text.substr(_off, tMetrics.length);
				}
			}

			return sText;
		}

		void InsertTextAt(const wchar_t *nText, UINT32 iIndex = -1, UINT32 nLen = -1)
		{
			if (nLen == -1)
				nLen = wcsnlen_s(nText, -1);
			
			if (selectionMetrics.size())
			{
				UINT32 tPos = (*selectionMetrics.begin()).textPosition;
				if (tPos < iIndex)
					iIndex = tPos;
				tPos = (*selectionMetrics.end()).textPosition;
				if (tPos < iIndex)
					iIndex = tPos;

				ClearSelection();
				cursor += nLen;
			}

			size_t tLen = text.length();
			if (iIndex > tLen)
				text += nText;
			else
				text = text.substr(0, iIndex) + nText + text.substr(iIndex, tLen - iIndex);
			length += nLen;

			UpdateText();
			UpdateCursor();
		}

		void MoveTo(unsigned int textPosition)
		{
			cursor = textPosition;
			UpdateCursor();
		}
		void MoveToPosition(D2D1_POINT_2F position)
		{
			if (textLayout != nullptr)
			{
				BOOL isTrailingHit, isInside;
				DWRITE_HIT_TEST_METRICS hitTestMetrics;
				textLayout->HitTestPoint(position.x - 2, position.y, &isTrailingHit, &isInside, &hitTestMetrics);
				if (isTrailingHit || hitTestMetrics.left)
				{
					cursorX = hitTestMetrics.left + hitTestMetrics.width;
					//	cursorY = hitTestMetrics.top - hitTestMetrics.height;
					if (text[hitTestMetrics.textPosition] == '\r' || text[hitTestMetrics.textPosition] == '\n')
						cursor = hitTestMetrics.textPosition;
					else
						cursor = hitTestMetrics.textPosition + 1;
				}
				else
				{
					cursorX = 0;
					cursor = hitTestMetrics.textPosition;
				}
				cursorY = hitTestMetrics.top;
				textHeight = hitTestMetrics.height;
			}
		}

		void PointerEntered(D2D1_POINT_2F* pPosition, short pState)
		{
			if (brushIndex != 2)
				brushIndex = 1;

#ifdef _UWP
			Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::IBEAM, 0);
#else
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
#endif

			Control::PointerEntered(pPosition, pState);
		}
		void PointerExited(D2D1_POINT_2F* pPosition, short pState)
		{
			if (brushIndex != 2)
				brushIndex = 0;


#ifdef _UWP
			Windows::UI::Core::CoreWindow::GetForCurrentThread()->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Arrow, 0);
#else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
#endif
			Control::PointerExited(pPosition, pState);
		}
		void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
			brushIndex = 2;
			CancelSelection();
			MoveToPosition(GetPositionForCurrent(*pPosition));
			Control::PointerPressed(pPosition, pState);
		}
		void PointerMoved(D2D1_POINT_2F* pPosition, short pState)
		{
			if (pointerState == 2 && textLayout != nullptr)
				SelectPosition(GetPositionForCurrent(*pPosition));
			
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
			Control::PointerMoved(pPosition, pState);
		}

		void Select(UINT32 textPosition)
		{
			if (textPosition == cursor)
			{
				CancelSelection();
				return;
			}

			UINT32 startPos, len, mCount;

			if (textPosition > cursor)
			{
				startPos = cursor;
				len = textPosition - cursor;
			}
			else
			{
				startPos = textPosition;
				len = cursor - textPosition;
			}
			D2D1_POINT_2F _offset = GetOffset();
			textLayout->HitTestTextRange(startPos, len, _offset.x, _offset.y, NULL, 0, &mCount);
			if (mCount)
			{
				selectionMetrics.resize(mCount);
				textLayout->HitTestTextRange(startPos, len, _offset.x, _offset.y, &selectionMetrics.front(), mCount, &mCount);
			}
		}
		void SelectPosition(D2D1_POINT_2F selPosition)
		{
			BOOL isTrailingHit, isInside;
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			textLayout->HitTestPoint(selPosition.x, selPosition.y, &isTrailingHit, &isInside, &hitTestMetrics);
			//				textLayout->HitTestTextPosition(hitTestMetrics.textPosition, isTrailingHit, &pX, &pY, &hitTestMetrics);
			Select(isTrailingHit || hitTestMetrics.left ? hitTestMetrics.textPosition + 1 : hitTestMetrics.textPosition);
		}
		void CancelSelection()
		{
			if (selectionMetrics.size())
				selectionMetrics.clear();
		}
		bool ClearSelection()
		{
			size_t _size = selectionMetrics.size();
			if (_size)
			{
				size_t _offset = 0, _off;
				for (size_t i = 0; i < _size; i++)
				{
					DWRITE_HIT_TEST_METRICS& tMetrics = selectionMetrics[i];
					_off = tMetrics.textPosition - _offset;
					text = text.substr(0, _off) + text.substr(_off + tMetrics.length, length - _off - tMetrics.length);
					_offset += tMetrics.length;
					length -= tMetrics.length;
				}
				cursor = (UINT32)_off;
				UpdateText();
				UpdateCursor();
				CancelSelection();

				return true;
			}

			return false;
		}

		void SizeChanged(D2D1_SIZE_F newSize)
		{
			UpdateText();
		}

		D2D1_SIZE_F GetRealSize()
		{
			return { textMetrics.widthIncludingTrailingWhitespace + 5.0f , textMetrics.height + 10.0f };
		}
	private:
		std::wstring text = L"";
		IDWriteTextLayout* textLayout = nullptr, *drawingTextLayout = nullptr;
		ID2D1Layer* d2dLayer;
		std::vector<DWRITE_HIT_TEST_METRICS> selectionMetrics;

		bool isTextLayoutLocked = false;

		DWRITE_TEXT_METRICS textMetrics = {};
		void UpdateText()
		{
			isTextLayoutLocked = true;
			SafeRelease((IUnknown**)&textLayout);
			murrela->wrtFactory->CreateTextLayout(text.c_str(), (UINT32)length, murrela->txtFormat.Get(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), &textLayout);
//			textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
			textLayout->GetMetrics(&textMetrics);

			DWRITE_TEXT_RANGE textRange;
			DWRITE_HIT_TEST_METRICS hitTestMetrics;
			BOOL isTrailingHit, isInside;
			textLayout->HitTestPoint(textMetrics.left + offsetX, textMetrics.top + offsetY, &isTrailingHit, &isInside, &hitTestMetrics);
			textRange.startPosition = hitTestMetrics.textPosition;
			textLayout->HitTestPoint(textMetrics.left + offsetX + ControlSize.width, textMetrics.top + offsetY + ControlSize.height, &isTrailingHit, &isInside, &hitTestMetrics);
			textRange.length = hitTestMetrics.textPosition + 1 - textRange.startPosition;

			SafeRelease((IUnknown**)&drawingTextLayout);
			murrela->wrtFactory->CreateTextLayout(text.substr(textRange.startPosition, textRange.length).c_str(), (UINT32)textRange.length, murrela->txtFormat.Get(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), &drawingTextLayout);
			
			isTextLayoutLocked = false;
//			SizeRequest({ textMetrics.widthIncludingTrailingWhitespace + 5 , textMetrics.height + 10 });
		}
		void UpdateCursor()
		{
			if (cursor)
			{
				DWRITE_HIT_TEST_METRICS cursorMetrics;
				D2D1_POINT_2F _offset;
				textLayout->HitTestTextPosition(cursor, false, &_offset.x, &_offset.y, &cursorMetrics);
				cursorX = cursorMetrics.left;
				cursorY = cursorMetrics.top;
				textHeight = cursorMetrics.height;
			}
			else
			{
				cursorX = 0;
				cursorY = 0;
			}
		}

		D2D1_POINT_2F GetOffset()
		{
			return D2D1::Point2F(ControlOffset.x + 2 + offsetX, ControlOffset.y + 5 + offsetY);
		}

		unsigned int cursor = 0, length = 0;
		float cursorX = 0.0f, cursorY = 0.0f, offsetX = 0.0f, offsetY = 0.0f, textHeight = 0.0f;
		short brushIndex = 0;
	protected:
	};

	class Container : public Control
	{
	public:
		Container(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}) :Control(murla, alignment, controlSize)
		{
			murrela->d2dContext->CreateLayer(&d2dLayer);
		}
		~Container()
		{
			SafeRelease((IUnknown**)&d2dLayer);
		}
		
	protected:
		ID2D1Layer* d2dLayer;
	};

	class ChildContainer : public Container
	{
	public:
		ChildContainer(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}) :Container(murla, alignment, controlSize)
		{

		}
		~ChildContainer()
		{
			if (child != nullptr)
			{
				delete child;
				child = nullptr;
			}
		}

		void Draw()
		{
			if (child != nullptr)
			{
				murrela->d2dContext->PushLayer(D2D1::LayerParameters1(GetRectForRender()), d2dLayer);

				auto pos = child->ControlOffset;
				pos = { pos.x + child->ControlSize.width, pos.y + child->ControlSize.height };
				if (!IsInside(&child->ControlOffset) || !IsInside(&pos))
					child->UpdateLayout();

				child->Draw();

				murrela->d2dContext->PopLayer();
			}
		}

		void SetContent(Control* newContent)
		{
			(child = newContent)->SetParent(this);
			SizeRequest({ child->ControlSize.width > minWidth ? child->ControlSize.width : minWidth, child->ControlSize.height > minHeight ? child->ControlSize.height : minHeight });
		}

		void PointerMoved(D2D1_POINT_2F* pPosition, short pState)
		{
			if (child != nullptr && child->IsInside(pPosition))
				child->PointerMoved(pPosition, pState);
			Control::PointerMoved(pPosition, pState);
		}

		void PointerExited(D2D1_POINT_2F* pPosition, short pState)
		{
			if (child != nullptr)
				child->PointerExited(pPosition, pState);
			Control::PointerExited(pPosition, pState);
		}

		void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
			if (child != nullptr && child->IsInside(pPosition))
				child->PointerPressed(pPosition, pState);
			Control::PointerPressed(pPosition, pState);
		}

		void PointerReleased(D2D1_POINT_2F* pPosition, short pState)
		{
			if (child != nullptr && child->IsInside(pPosition))
				child->PointerReleased(pPosition, pState);
			Control::PointerReleased(pPosition, pState);
		}

		void UpdateLayout()
		{
			Control::UpdateLayout();

			if (child != nullptr)
				child->UpdateLayout();
		}

		void UpdateSize()
		{
			SizeRequest({ child->ControlSize.width > minWidth ? child->ControlSize.width : minWidth, child->ControlSize.height > minHeight ? child->ControlSize.height : minHeight });
		}
	protected:
		Control* child = nullptr;
	};

	class ItemsContainer : public Container
	{
	public:
		ItemsContainer(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}) : Container(murla, alignment, controlSize)
		{
		}
		~ItemsContainer()
		{
		}

		virtual void AppendItem(Control* newItem, size_t index = -1)
		{
			if (index == -1)
				items.push_back(newItem);
			else if (index < items.size())
				items.insert(items.begin() + index, newItem);
			newItem->SetParent(this);
		}
		virtual void RemoveItem(Control* target)
		{
			std::vector<Control*>::iterator tItem = items.begin(), eItem = items.end();
			while (tItem != eItem)
			{
				if (*tItem._Ptr == target)
				{
					items.erase(tItem);
					break;
				}
				tItem++;
			}
		}

		virtual void Draw()
		{
			murrela->d2dContext->PushLayer(D2D1::LayerParameters1(GetRectForRender()), d2dLayer);

			size_t _size = GetItemsNum();
			for (size_t i = 0; i < _size; i++)
				items[i]->Draw();

			murrela->d2dContext->PopLayer();
		}

		size_t GetItemsNum()
		{
			return items.size();
		}

		Control* GetAt(UINT32 index)
		{
			return index < items.size() ? items[index] : nullptr;
		}
		void RemoveAt(UINT32 index)
		{
			if (index < items.size())
			{
				auto it = items.begin();
				std::advance(it, index);
				items.erase(it);
			}
		}
		void PointerMoved(D2D1_POINT_2F* pPosition, short pState);
		void PointerExited(D2D1_POINT_2F* pPosition, short pState)
		{
			size_t _size = GetItemsNum();
			if (pointerIndex < _size)
			{
				items[pointerIndex]->PointerExited(pPosition, pState);
				pointerIndex = -1;
			}

			Control::PointerExited(pPosition, pState);
		}
		void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
			size_t _size = GetItemsNum();
			if (_size)
			{
				size_t i;
				for (i = 0; i < _size; i++)
					if (items[i]->IsInside(pPosition))
						{
							items[i]->PointerPressed(pPosition, pState);
							return;
						}
			}
			Control::PointerPressed(pPosition, pState);
		}
		void PointerReleased(D2D1_POINT_2F* pPosition, short pState);

		virtual void UpdateLayout()
		{
			Control::UpdateLayout();
			size_t itemsCount = GetItemsNum();

			for (size_t i = 0; i < itemsCount; i++)
				items[i]->UpdateLayout();
		}

	private:
		size_t pointerIndex = -1;
	protected:
		std::vector<Control*> items;
		D2D1_POINT_2F lastPos = { -1.0f, -1.0f };

	};

	typedef struct
	{
		short state; //0 Max 1 Min 2 Manual
		float offset;
		float size;
	} GridBlock;

	class Grid : public ItemsContainer
	{
	public:
		Grid(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}) : ItemsContainer(murla, alignment, controlSize)
		{

		}

		virtual void AppendItem(Control* newItem, int colNum = 0, int rowNum = 0)
		{
			size_t itemIndex = items.size();
			ItemsContainer::AppendItem(newItem);
			SetItemPosition(0, itemIndex, colNum);
			SetItemPosition(1, itemIndex, rowNum);

			UpdateLayout();
		}
		void AppendPos(int isRow, GridBlock newBlock = {})
		{
			pos[isRow].push_back(newBlock);
			UpdatePos(isRow);
		}

		void SetItemPosition(int isRow, size_t itemIndex, int posNum)
		{
			if (itemsPos[isRow].size() <= itemIndex)
				itemsPos[isRow].resize(itemIndex + 1);
			itemsPos[isRow][itemIndex] = posNum;

			itemIndex = pos[isRow].size();
			while (posNum >= (int)itemIndex++)
				AppendPos(isRow);
		}

		void UpdateLayout()
		{
			Control::UpdateLayout();
			size_t itemsCount = GetItemsNum();

			UpdatePos(0);
			UpdatePos(1);

			auto _offset = ControlOffset;
			auto _size = ControlSize;
			GridBlock* tColumn, * tRow;
			for (size_t i = 0; i < itemsCount; i++)
			{
				tColumn = &pos[0][itemsPos[0][i]],
					tRow = &pos[1][itemsPos[1][i]];
					
				ControlOffset = { tColumn->offset, tRow->offset };
				ControlSize = {tColumn->size, tRow->size};

				items[i]->UpdateLayout();
			}
			ControlOffset = _offset;
			ControlSize = _size;
		}
	private:
		void UpdatePos(int isRow)
		{
			size_t i, _size = pos[isRow].size(), maxCount = 0;
			float maxSize = isRow ? ControlSize.height : ControlSize.width;
			size_t* maxIndexes = new size_t[_size];
			for (i = 0; i < _size; i++)
			{
				GridBlock& tItem = pos[isRow][i];
				if (tItem.state)
					maxSize -= tItem.size;
				else
					maxIndexes[maxCount++] = i;
			}

			maxSize /= maxCount;
			for (i = 0; i < maxCount; i++)
				pos[isRow][maxIndexes[i]].size = maxSize;
			delete[] maxIndexes;
			maxIndexes = nullptr;

			float _offset = ((float*)&ControlOffset)[isRow];
			for (i = 0; i < _size; i++)
			{
				pos[isRow][i].offset = _offset;
				_offset += pos[isRow][i].size;
			}
		}
		std::vector<GridBlock> pos[2];
		std::vector<int> itemsPos[2];
	};

	class StackPanel : public ItemsContainer
	{
	public:
		StackPanel(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}, bool orientation = true) : ItemsContainer(murla, alignment, controlSize)
		{
			Orientation = orientation;
		}

		//False Horizontal True Vertical
		bool Orientation;

		virtual void AppendItem(Control* newItem, size_t index = -1) override
		{
			ItemsContainer::AppendItem(newItem, index);
			this->UpdateLayout();
		}

		void UpdateLayout()
		{
			Control::UpdateLayout();
			size_t _size = GetItemsNum();
			if (_size)
			{
				size_t i;

				float tSize[2], * iSize, * iOffset, * cSize = (float*)&ControlSize, * cOffset = (float*)&ControlOffset;
				const int o = (int)Orientation;
				const int invO = o ^ 1;
				tSize[o] = cOffset[o];
				tSize[invO] = 0;
				for (i = 0; i < _size; i++)
				{
					iSize = (float*)&items[i]->ControlSize;
					iOffset = (float*)&items[i]->ControlOffset;
					iOffset[o] = tSize[o];
					if (iOffset[invO] < cOffset[invO] || iOffset[invO] + iSize[invO] > cOffset[invO] + cSize[invO])
						iOffset[invO] = cOffset[invO];

					tSize[o] += iSize[o];
					if (iSize[invO] > tSize[invO])
						tSize[invO] = iSize[invO];
				}
				cSize[o] = tSize[o] - cOffset[o];

				if (tSize[invO] != cSize[invO])
					cSize[invO] = tSize[invO];

				for (i = 0; i < _size; i++)
					switch (items[i]->Alignment)
					{
					case Center:
						((float*)&items[i]->ControlOffset)[invO] = cOffset[invO] + (cSize[invO] - ((float*)&items[i]->ControlSize)[invO]) / 2;
						break;
					case Stretch:
						((float*)&items[i]->ControlSize)[invO] = cSize[invO];
					case Top:
						((float*)&items[i]->ControlOffset)[invO] = cOffset[invO];
						break;
					case Bottom:
						((float*)&items[i]->ControlOffset)[invO] = cOffset[invO] + cSize[invO] - ((float*)&items[i]->ControlSize)[invO];
						break;
					}
			}
		}
	};

	class Button : public ChildContainer
	{
	public:
		Button(const wchar_t* text, Murrela* murla, Alignments alignment, D2D1_SIZE_F buttonSize = {}) :ChildContainer(murla, alignment, buttonSize)
		{
			SetContent((Control*)new TextBlock(text, murrela, Center, Center));
			Init();
			ClickEvent.events = (PVOID)&Clicked;
		}

		void Draw()
		{
//			murrela->d2dContext->DrawRectangle(ControlRect, brushes[pointerState]);
			murrela->d2dContext->FillRectangle(GetRectForRender(), brushes[pointerState]);
			ChildContainer::Draw();
		}

		std::vector<void(*)(void* param)> Clicked;
		NormalEvent ClickEvent;

		void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
		//	SetText(L"Pressed");
			if (pState & 4)
				ChildContainer::PointerPressed(pPosition, pState);
		}
		void PointerReleased(D2D1_POINT_2F* pPosition, short pState)
		{
			//clicked
		//	SetText(L"Released");
			if (pointerState == 2)
			{
				if (Control::IsInside(pPosition))
				{
					CallEvent(&ClickEvent);
				}
				ChildContainer::PointerReleased(pPosition, pState);
			}
		}
		void SetText(const wchar_t* newText)
		{
			((TextBlock*)child)->SetText(newText);
			UpdateSize();
		}
		const wchar_t* GetText()
		{
			return ((TextBlock*)child)->GetText();
		}
	};

	class RepeatButton : public Button
	{
	public:
		RepeatButton(const wchar_t* text, Murrela* murla, Alignments alignment, D2D1_SIZE_F buttonSize = {}) :Button(text, murla, alignment, buttonSize)
		{

		}
		~RepeatButton()
		{
		}

		void PointerPressed(D2D1_POINT_2F* pPosition, short pState);
		void PointerReleased(D2D1_POINT_2F* pPosition, short pState);
	private:
		static void OnClicking(PVOID context)
		{
			auto clicked = (std::vector<void(*)(void* param)>*)context;
			size_t count = clicked->size();
			for (size_t i = 0; i < count; i++)
				(*clicked)[i](((NormalEvent*)context)->param);
		}
	};

	class Slider : public ChildContainer
	{
	public:
		Slider(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}, bool _orientation = true) :ChildContainer(murla, alignment, controlSize)
		{
			Init();
			orientation = _orientation;
			SetContent(new Button(L"=", murla, Stretch));
			cOffset = (float*)&child->ControlOffset, cSize = (float*)&child->ControlSize, mOffset = (float*)&ControlOffset, mSize = (float*)&ControlSize;
		}
		void Draw()
		{
			int o = (int)orientation, invO = (int)!orientation;
			cOffset[invO] = mOffset[invO];
			cSize[invO] = mSize[invO];
			cSize[o] = GetSliderSizeForRender();
			cOffset[o] = Value * mSize[o] + mOffset[o];
			if (cOffset[o] + cSize[o] > mOffset[o] + mSize[o])
				cOffset[o] = mOffset[o] + mSize[o] - cSize[o];
			child->Draw();
		}

		void PointerMoved(D2D1_POINT_2F* pPosition, short pState)
		{
			ChildContainer::PointerMoved(pPosition, pState);
			if (pointerState == 2)
			{
				int o = (int)orientation;
				float* pos = (float*)pPosition;
				SetValue((pos[o] - GetSliderSizeForRender() / 2 - mOffset[o]) / mSize[o]);
			}
		}

		void PointerPressed(D2D1_POINT_2F* pPosition, short pState)
		{
			ChildContainer::PointerPressed(pPosition, pState);
			int o = (int)orientation;
			float* pos = (float*)pPosition;
			SetValue((pos[o] - GetSliderSizeForRender() / 2 - mOffset[o]) / mSize[o]);
		}

		void SetValue(float newValue)
		{
			if (newValue < MinValue)
				newValue = MinValue;
			else if (newValue > MaxValue)
				newValue = MaxValue;

			Value = newValue;
		}

		void UpdateLayout()
		{
			Control::UpdateLayout();
			int invO = (int)orientation ^ 1;
			cOffset[invO] = mOffset[invO];
		}

		float GetSliderSizeForRender()
		{
			return mSize[(int)orientation] < MaxValue ? mSize[(int)orientation] * mSize[(int)orientation] / MaxValue : mSize[(int)orientation];
		}

		float Value = 0.0f;
		float MinValue = 0.0f;
		float MaxValue = 1.0f;
	protected:
		bool orientation;
		float* cOffset, * cSize, * mOffset, * mSize;
	};

	class ScrollBar : StackPanel
	{
	public:
		ScrollBar(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}, bool _orientation = true) :StackPanel(murla, alignment, controlSize, _orientation)
		{
			AppendItem(decreaseBtn = new RepeatButton(L"", murla, Stretch));
			AppendItem(mSlider = new Slider(murla, Stretch, {}, _orientation));
			AppendItem(increaseBtn = new RepeatButton(L"", murla, Stretch));

			SetOrientation(_orientation);

			decreaseBtn->ClickEvent.param = mSlider;
			increaseBtn->ClickEvent.param = mSlider;
			decreaseBtn->Clicked.push_back((void (*)(void* param)) & dBtn_Clicked);
			increaseBtn->Clicked.push_back((void (*)(void* param)) & iBtn_Clicked);
		}

		void SetOrientation(bool _orientation)
		{
			if (_orientation)
			{
				decreaseBtn->SetText(L"⏶");
				increaseBtn->SetText(L"⏷");
			}
			else
			{
				decreaseBtn->SetText(L"⏴");
				increaseBtn->SetText(L"⏵");
			}
			Orientation = _orientation;
		}

		void UpdateLayout()
		{
			if (HAS_PARENT && items.size() == 3)
			{
				int invO = (int)Orientation;
				((float*)&items[1]->ControlSize)[invO] = ((float*)&Parent->ControlSize)[invO] - ((float*)&items[0]->ControlSize)[invO] * 2;
			}
			StackPanel::UpdateLayout();
		}

		void SetMaxValue(float newMaxValue)
		{
			mSlider->MaxValue = newMaxValue;
		}
	protected:
		RepeatButton* decreaseBtn, * increaseBtn;
		static void dBtn_Clicked(Slider* slider)
		{
			slider->SetValue(slider->Value - 0.01f);
		}
		static void iBtn_Clicked(Slider* slider)
		{
			slider->SetValue(slider->Value + 0.01f);
		}
		Slider* mSlider;
	};

	class ScrollViewer : Grid
	{
	public:
		ScrollViewer(Murrela* murla, Alignments alignment, D2D1_SIZE_F controlSize = {}) : Grid(murla, alignment, controlSize)
		{
			AppendPos(0);
			AppendPos(1);
			AppendPos(0, {(short)1, 0.0f, 20.0f});
			AppendPos(1, { (short)1, 0.0f, 20.0f });
			AppendItem((Control*)(hScrollBar = new ScrollBar(murla, Stretch)), 1, 0);
			AppendItem((Control*)(vScrollBar = new ScrollBar(murla, Stretch, {}, false)), 0, 1);
		}

		void SetContent(Control* _content)
		{
			if (items.size() == 3)
				RemoveItem(*items.end());
			AppendItem(_content);
		}

		Control* GetContent()
		{
			return items.size() == 3 ? items[2] : nullptr;
		}

		void UpdateLayout()
		{
			Grid::UpdateLayout();
			UpdateScrollBar();
		}

		void UpdateScrollBar()
		{
			Control* content = GetContent();
			if (content != nullptr)
			{
				D2D1_SIZE_F realSize = content->GetRealSize();
				hScrollBar->SetMaxValue(realSize.height);
				vScrollBar->SetMaxValue(realSize.width);
			}
		}

	private:
		ScrollBar* hScrollBar, *vScrollBar;
	};

	class Tab : public ItemsContainer
	{
	public:
		Tab(const wchar_t* title, Murrela* murla, Alignments alignment, D2D1_SIZE_F blockSize = {}) : ItemsContainer(murla, alignment, blockSize)
		{
			this->PageContent = nullptr;
			ControlSize.width = 115;
			ControlSize.height = 30;
			this->AppendItem((Control*)(TabTitle = new TextBlock(title, murla, Center, Left | Center, {90.0f, 0.0f})));
			this->AppendItem((Control*)(ClsBtn = new Button(L"x", murla, Right | Center, {20.0f, 0.0f})));
			Init();
		}

		void Draw()
		{
			float offsetY = this->ControlOffset.y + (this->ControlSize.height - ((Control*)TabTitle)->ControlSize.height) / 2;
			((Control*)TabTitle)->ControlOffset = { this->ControlOffset.x, offsetY };
			((Control*)ClsBtn)->ControlOffset = { this->ControlOffset.x + 90, offsetY };
//			((Control*)TabTitle)->UpdateLayout();
//			((Control*)ClsBtn)->UpdateLayout();
			if (brushes[pointerState] != nullptr)
				murrela->d2dContext->FillRectangle(GetRectForRender(), brushes[IsSelected() ? 2 : pointerState]);
		//	murrela->DrawShadow(GetRectForRender());
			ItemsContainer::Draw();
		}

		void PointerPressed(D2D1_POINT_2F* pPosition, short pState);

		void PointerEntered(D2D1_POINT_2F* pPosition, short pState)
		{
			if (IsInside(pPosition))
				pointerState = 1;
		}

		bool IsSelected();

		TextBlock* TabTitle;
		Control* PageContent;
		Button* ClsBtn;
	};

	class TabContainer : public StackPanel
	{
	public:
		TabContainer(Murrela* murla, Alignments alignment, D2D1_SIZE_F blockSize = {}) : StackPanel(murla, alignment, blockSize, false)
		{
		}

		static void CloseTab(Tab* thisTab)
		{
			TabContainer* tC = (TabContainer*)thisTab->Parent;
			for (std::vector<Control*>::iterator i = tC->items.begin(); i != tC->items.end(); i++)
			{
				if (*i == thisTab)
				{
					delete thisTab->PageContent;
					thisTab->PageContent = nullptr;
					tC->items.erase(i);
					tC->UpdateLayout();
					break;
				}
			}
		}

		virtual void AppendItem(Tab* newTab, size_t index = -1)
		{
			newTab->ClsBtn->ClickEvent.param = newTab;
			newTab->ClsBtn->Clicked.push_back((void(*)(void* param))&TabContainer::CloseTab);
			StackPanel::AppendItem(newTab, index);
		}

		Tab* SelectedTab = nullptr;
		void Select(Tab* targetTab)
		{
			SelectedTab = targetTab;
		}

		void SelectAt(size_t tabIndex)
		{
			if (tabIndex < items.size())
				Select((Tab*)items[tabIndex]);
		}

	};
	

	bool _IsControlFocused(Control* tControl);
	void _CharacterReceived(unsigned int keycode);
	void _KeyReceived(unsigned int keycode, bool isReleased);

	DWORD _ReDrawLoop(_In_ LPVOID content);
	void _StartReDrawLoop(Control** content);
	void _StopReDrawLoop();
	void _Drew();
	/*
	namespace XML
	{
		const wchar_t ControlsName[] = L"Grid\0StackPanel\0Button\0RepeatButton\0Image\0TextBlock\0TextBox";
		const unsigned char ControlIndex[] = {0, 5, 16, 23, 36, 42, 52};
//		static void XMLToControls(const wchar_t* xmlStr, long xmlLen, Murrela* murrela, Control** result);
//		static void XMLToCode(const wchar_t* xmlStr, long xmlLen, CoreApp* murrela, std::wstring* result);
	}*/
}