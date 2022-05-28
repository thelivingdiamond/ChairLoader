/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Strings.h"
#include <algorithm>
#include "ww/PropertyTree/TreeImpl.h"
#include "ww/Window.h"
#include "ww/Entry.h"

#include "ww/_WidgetWindow.h"
#include "ww/Win32/Handle.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"
#include "ww/Unicode.h"
#include "ww/Win32/Window32.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Win32/CommonControls.h"
#include "gdiplusUtils.h"
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyRow.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "wwDrawContext.h"
#include "ww/PropertyTree.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

struct DebugTimer
{
	const char* name;
	unsigned int startTime;
	DebugTimer(const char* name)
	: name(name)
	{
		startTime = timeGetTime();
	}

	~DebugTimer()
	{
		unsigned int endTime = timeGetTime();
		char buf[128] = "";
		sprintf_s(buf, "timer %s: %i\n", name, endTime-startTime);
		// OutputDebugStringA(buf);
	}
};

using namespace Gdiplus;

namespace ww{

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
DragWindow::DragWindow(TreeImpl* treeImpl)
: treeImpl_(treeImpl)
, offset_(0, 0)
{
	DWORD exStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT;
	WW_VERIFY(create(L"", WS_POPUP | WS_DISABLED, Rect(0, 0, 320, 320), 0, exStyle));

}
#pragma warning(pop)

void DragWindow::set(TreeImpl* treeImpl, PropertyRow* row, const Rect& rowRect)
{
	RECT rect;
	GetClientRect(treeImpl_->handle(), &rect);
	ClientToScreen(treeImpl_->handle(), (POINT*)&rect);
	ClientToScreen(treeImpl_->handle(), (POINT*)&rect + 1);

	offset_.x = rect.left;
	offset_.y = rect.top;

	row_ = row;
	rect_ = rowRect;
}

void DragWindow::setWindowPos(bool visible)
{
	SetWindowPos(handle(), HWND_TOP, rect_.left() + offset_.x,  rect_.top() + offset_.y,
				 rect_.width(), rect_.height(), 
				 SWP_NOOWNERZORDER | (visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW)| SWP_NOACTIVATE);
}

void DragWindow::show()
{
	updateLayeredWindow();
	setWindowPos(true);
	updateLayeredWindow();
}

void DragWindow::move(int deltaX, int deltaY)
{
	offset_ += Vect2(deltaX, deltaY);
	setWindowPos(::IsWindowVisible(handle()) ? true : false);
}

void DragWindow::hide()
{
	setWindowPos(false);
}

struct DrawRowVisitor
{
	DrawRowVisitor(Gdiplus::Graphics* graphics) : graphics(graphics) {}

	ScanResult operator()(PropertyRow* row, ::PropertyTree* tree, int index)
	{
		wwDrawContext context((PropertyTree*)tree, graphics);
		if (row->pulledUp() || (row->parent() && row->parent()->pulledUp())) {
			row->drawRow(context, tree, index, true);
			row->drawRow(context, tree, index, false);
		}

		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	Gdiplus::Graphics* graphics;
};

void DragWindow::drawRow(HDC dc)
{
	Rect entireRowRect(0, 0, rect_.width(), rect_.height());

	HGDIOBJ oldBrush = ::SelectObject(dc, GetSysColorBrush(COLOR_BTNFACE));
	HGDIOBJ oldPen = ::SelectObject(dc, GetStockObject(WHITE_PEN));
	Rectangle(dc, entireRowRect.left(), entireRowRect.top(), entireRowRect.right(), entireRowRect.bottom());
	::SelectObject(dc, oldBrush);
	::SelectObject(dc, oldPen);

	Vect2 leftTop(row_->rect().left(), row_->rect().top());
	Gdiplus::Graphics graphics(dc);
	SetViewportOrgEx(dc, (leftTop.x + treeImpl_->tree()->tabSize()), leftTop.y, 0);
	int index = 0;
	if (row_->parent())
		index = row_->parent()->childIndex(row_);
	wwDrawContext context(treeImpl_->tree(), &graphics);
	row_->drawRow(context, treeImpl_->tree(), index, true);
	row_->drawRow(context, treeImpl_->tree(), index, false);
	row_->scanChildren(DrawRowVisitor(&graphics), treeImpl_->tree());
	SetViewportOrgEx(dc, 0, 0, 0);
}

BOOL DragWindow::onMessageEraseBkgnd(HDC dc)
{
	return TRUE;
}

void DragWindow::updateLayeredWindow()
{
	BLENDFUNCTION blendFunction;
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.BlendFlags = 0;
	blendFunction.SourceConstantAlpha = 255;
	blendFunction.AlphaFormat = 0;	

	PAINTSTRUCT ps;
	HDC realDC = BeginPaint(handle(), &ps);
	{
		Win32::MemoryDC dc(realDC);
		drawRow(dc);

		POINT leftTop = { rect_.left() + offset_.x, rect_.top() + offset_.y };
		SIZE size = { rect_.width(), rect_.height() };
		POINT point = { 0, 0 };
		UpdateLayeredWindow(handle(), realDC, &leftTop, &size, dc, &point, 0, &blendFunction, ULW_ALPHA);
		SetLayeredWindowAttributes(handle(), 0, 192, ULW_ALPHA);
	}
	EndPaint(handle(), &ps);
}

void DragWindow::onMessagePaint()
{
}

// ---------------------------------------------------------------------------

DragController::DragController(TreeImpl* treeImpl)
: treeImpl_(treeImpl)
, captured_(false)
, dragging_(false)
, before_(false)
, row_(0)
, clickedRow_(0)
, window_(treeImpl)
, hoveredRow_(0)
, destinationRow_(0)
{
}

void DragController::beginDrag(PropertyRow* clickedRow, PropertyRow* draggedRow, POINT pt)
{
	row_ = draggedRow;
	clickedRow_ = clickedRow;
	startPoint_ = pt;
	lastPoint_ = pt;
	captured_ = true;
	dragging_ = false;
}

bool DragController::dragOn(POINT screenPoint)
{
	if (dragging_)
		window_.move(screenPoint.x - lastPoint_.x, screenPoint.y - lastPoint_.y);

	bool needCapture = false;
	if(!dragging_ && (Vect2(startPoint_.x, startPoint_.y) - Vect2(screenPoint.x, screenPoint.y)).length2() >= 25)
		if(row_->canBeDragged()){
			needCapture = true;
			property_tree::Rect area = treeImpl_->tree()->area();
			property_tree::Point offset = treeImpl_->tree()->offset();
			property_tree::Rect rect = row_->rect();
			rect.y += area.top();
			rect = property_tree::Rect(rect.left() - offset.x() + treeImpl_->tree()->tabSize(),
				rect.top() - offset.y(), rect.width() - +treeImpl_->tree()->tabSize(), rect.height() + 1);
            
			window_.set(treeImpl_, row_, ww::Rect(rect.left(), rect.top(), rect.right(), rect.bottom()));
			window_.move(screenPoint.x - startPoint_.x, screenPoint.y - startPoint_.y);
			window_.show();
			dragging_ = true;
		}

	if(dragging_){
		POINT point = screenPoint;
		ScreenToClient(treeImpl_->handle(), &point);
		trackRow(point);
	}
	lastPoint_ = screenPoint;
	return needCapture;
}

void DragController::interrupt()
{
	captured_ = false;
	dragging_ = false;
	row_ = 0;
	window_.hide();
}

void DragController::trackRow(POINT pt)
{
	hoveredRow_ = 0;
	destinationRow_ = 0;

    property_tree::Point point(pt.x, pt.y);
	PropertyRow* row = treeImpl_->tree()->rowByPoint(point);
	if(!row || !row_)
		return;

	row = row->nonPulledParent();
	if(!row->parent() || row->isChildOf(row_) || row == row_)
		return;

	PropertyTree* tree = treeImpl_->tree();
	float pos = (point.y() - row->rect().top() - treeImpl_->tree()->area().top()) / float(row->rect().height());
	if(row_->canBeDroppedOn(row->parent(), row, tree)){
		if(pos < 0.25f){
			destinationRow_ = row->parent();
			hoveredRow_ = row;
			before_ = true;
			return;
		}
		if(pos > 0.75f){
			destinationRow_ = row->parent();
			hoveredRow_ = row;
			before_ = false;
			return;
		}
	}
	if(row_->canBeDroppedOn(row, 0, tree))
		hoveredRow_ = destinationRow_ = row;
}

void DragController::drawUnder(HDC dc)
{
	if(dragging_ && destinationRow_ == hoveredRow_ && hoveredRow_){
		property_tree::Rect rowRect = hoveredRow_->rect().adjusted(treeImpl_->tree()->tabSize(), 0, 0, 0);
		RECT rt = { rowRect.left(), rowRect.top(), rowRect.right(), rowRect.bottom() };
		FillRect(dc, &rt, GetSysColorBrush(COLOR_HIGHLIGHT));
	}
}

void DragController::drawOver(HDC dc)
{
	if(!dragging_)
		return;
	
	property_tree::Rect rr = row_->rect();
	Rect rowRect(rr.left(), rr.top(), rr.right(), rr.bottom());

	if(destinationRow_ != hoveredRow_ && hoveredRow_){
		const int tickSize = 4;
		property_tree::Point offset = treeImpl_->tree()->pointToRootSpace(property_tree::Point(0, 0));
		int ox = offset.x();
		int oy = offset.y();
		property_tree::Rect hoveredRect = hoveredRow_->rect().adjusted(treeImpl_->tree()->tabSize() - ox, -oy, -ox, -oy);

		if(!before_){ // previous
			RECT rect = { hoveredRect.left() - 1 , hoveredRect.bottom() - 1, hoveredRect.right() + 1, hoveredRect.bottom() + 1 };
			RECT rectLeft = { hoveredRect.left() - 1 , hoveredRect.bottom() - tickSize, hoveredRect.left() + 1, hoveredRect.bottom() + tickSize };
			RECT rectRight = { hoveredRect.right() - 1 , hoveredRect.bottom() - tickSize, hoveredRect.right() + 1, hoveredRect.bottom() + tickSize };
			FillRect(dc, &rect, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectLeft, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectRight, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
		else{ // next
			RECT rect = { hoveredRect.left() - 1 , hoveredRect.top() - 1, hoveredRect.right() + 1, hoveredRect.top() + 1 };
			RECT rectLeft = { hoveredRect.left() - 1 , hoveredRect.top() - tickSize, hoveredRect.left() + 1, hoveredRect.top() + tickSize };
			RECT rectRight = { hoveredRect.right() - 1 , hoveredRect.top() - tickSize, hoveredRect.right() + 1, hoveredRect.top() + tickSize };
			FillRect(dc, &rect, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectLeft, GetSysColorBrush(COLOR_HIGHLIGHT));
			FillRect(dc, &rectRight, GetSysColorBrush(COLOR_HIGHLIGHT));
		}
	}
}

void DragController::drop(POINT screenPoint)
{
	PropertyTreeModel* model = treeImpl_->tree()->model();
	if(row_ && hoveredRow_){
		YASLI_ASSERT(destinationRow_);
		clickedRow_->setSelected(false);
		row_->dropInto(destinationRow_, destinationRow_ == hoveredRow_ ? 0 : hoveredRow_, treeImpl_->tree(), before_);
	}

	captured_ = false;
	dragging_ = false;
	row_ = 0;
	window_.hide();
	hoveredRow_ = 0;
	destinationRow_ = 0;
}


// ---------------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
TreeImpl::TreeImpl(PropertyTree* tree)
: _ContainerWindow(tree)
, tree_(tree)
, redrawLock_(false)
, redrawRequest_(false)
{
	WW_VERIFY(create(L"", WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, Rect(0, 0, 40, 40), Win32::getDefaultWindowHandle()));
}
#pragma warning(pop)

TreeImpl::~TreeImpl()
{
	tree_ = 0;
}

int TreeImpl::onMessageKillFocus(HWND newFocusedWindow)
{
	RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	return Window32::onMessageKillFocus(newFocusedWindow);
}

int TreeImpl::onMessageSetFocus(HWND lastFocusedWindow)

{
    int result = 0;
	if(!creating()){
	    result = Window32::onMessageSetFocus(lastFocusedWindow);
		owner_->_setFocus();
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
    return result;
}

void TreeImpl::onMessageMouseWheel(SHORT delta)
{
	tree_->onMouseWheel(delta);
	__super::onMessageMouseWheel(delta);
}

void TreeImpl::onMessageLButtonDblClk(int x, int y)
{
	tree_->onLButtonDoubleClick(x, y);
	__super::onMessageLButtonDblClk(x, y);
}

void TreeImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	YASLI_ASSERT(::IsWindow(handle_));
	tree_->onLButtonUp(button, x, y);
}

void TreeImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	YASLI_ASSERT(::IsWindow(handle_));
	tree_->onLButtonDown(button, x, y);
}

void TreeImpl::onMessageRButtonDown(UINT button, int x, int y)
{
	YASLI_ASSERT(::IsWindow(handle_));
	tree_->onRButtonDown(button, x, y);
}

void TreeImpl::onMessageMButtonDown(UINT button, int x, int y)
{
	::SetFocus(handle_);
	__super::onMessageMButtonDown(button, x, y);
}

void TreeImpl::onMessageMouseMove(UINT button, int x, int y)
{
	tree_->onMouseMove(button, x, y);
}

BOOL TreeImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}

void TreeImpl::onMessageScroll(UINT message, WORD type)
{
	SetFocus(handle_);
	SCROLLINFO info;
	info.cbSize = sizeof(info);
	info.fMask  = SIF_ALL;

	HDC dc = GetDC(handle());
	YASLI_ASSERT(dc);
	HGDIOBJ oldFont = ::SelectObject(dc, Win32::defaultFont());
	TEXTMETRIC textMetric;
	WW_VERIFY(GetTextMetrics(dc, &textMetric));
	::SelectObject(dc, oldFont);
	int lineScrollHeight = textMetric.tmHeight + 2;
	ReleaseDC(handle(), dc);

	::GetScrollInfo(handle_, message == WM_VSCROLL ? SB_VERT : SB_HORZ, &info);
	int oldPosition = info.nPos;
	switch(type){
	case SB_TOP:        info.nPos = info.nMin; break;
	case SB_BOTTOM:     info.nPos = info.nMax; break;
	case SB_LINEUP:     info.nPos -= lineScrollHeight; break;
	case SB_LINEDOWN:   info.nPos += lineScrollHeight; break;
	case SB_PAGEUP:     info.nPos -= info.nPage; break;
	case SB_PAGEDOWN:   info.nPos += info.nPage; break;
	case SB_THUMBTRACK: info.nPos = info.nTrackPos; break;
	default:			break; 
	}

	info.fMask = SIF_POS;

	if(message == WM_VSCROLL){
		::SetScrollInfo(handle_, SB_VERT, &info, TRUE);
		::GetScrollInfo(handle_, SB_VERT, &info);
		tree_->onScroll(int(info.nPos));
	}


	if(info.nPos != oldPosition){
		tree_->_arrangeChildren();
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
		tree_->updateScrollBar();
	}
}

BOOL TreeImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(!creating()){
        tree()->updateHeights();
	}
	return TRUE;
}

void TreeImpl::onMessagePaint()
{
	if(redrawLock_){
		redrawRequest_ = true;
		return;
	}
	redrawRequest_ = false;

	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle_, &ps);
	{
		Win32::MemoryDC memoryDC(dc);
		tree_->onPaint(memoryDC);
	}
	EndPaint(handle_, &ps);
	__super::onMessagePaint();
}

static int translateKeyCode(unsigned int keyCode)
{
	switch (keyCode) {
	case VK_BACK: return property_tree::KEY_BACKSPACE;
	case VK_DELETE: return property_tree::KEY_DELETE;
	case VK_DOWN: return property_tree::KEY_DOWN;
	case VK_END: return property_tree::KEY_END;
	case VK_ESCAPE: return property_tree::KEY_ESCAPE;
	case VK_F2: return property_tree::KEY_F2;
	case VK_HOME: return property_tree::KEY_HOME;
	case VK_INSERT: return property_tree::KEY_INSERT;
	case VK_LEFT: return property_tree::KEY_LEFT;
	case VK_MENU: return property_tree::KEY_MENU;
	case VK_RETURN: return property_tree::KEY_RETURN;
	case VK_RIGHT: return property_tree::KEY_RIGHT;
	case VK_SPACE: return property_tree::KEY_SPACE;
	case VK_UP: return property_tree::KEY_UP;
	case 'C': return property_tree::KEY_C;
	case 'V': return property_tree::KEY_V;
	case 'Z': return property_tree::KEY_Z;
	}

	return property_tree::KEY_UNKNOWN;

}

static int translateModifiers(unsigned short flags)
{
}

static property_tree::KeyEvent translateKeyEvent(unsigned int keyCode, unsigned short flags)
{
	property_tree::KeyEvent e;
	e.key_ = translateKeyCode(keyCode);
	if (GetKeyState(VK_MENU) >> 15)
		e.modifiers_ |= MODIFIER_ALT;
	if (GetKeyState(VK_CONTROL) >> 15)
		e.modifiers_ |= MODIFIER_CONTROL;
	if (GetKeyState(VK_SHIFT) >> 15)
		e.modifiers_ |= MODIFIER_SHIFT;
	return e;		
}

int TreeImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
    KeyPress keyPress = KeyPress::addModifiers(Key(keyCode));

	if (keyPress == KeyPress(KEY_F, KEY_MOD_CONTROL)) {
        tree_->setFilterMode(true);
    }

    if (tree_->filterMode_) {
        if (keyPress == KeyPress(KEY_ESCAPE)) {
            tree_->setFilterMode(false);
        }
    }

	PropertyRow* row = tree_->model()->focusedRow();

	property_tree::KeyEvent keyEvent = translateKeyEvent(keyCode, flags);
	bool result = tree_->onRowKeyDown(row, &keyEvent);
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
	if(!result)
		return __super::onMessageKeyDown(keyCode, count, flags);
	else
		return 0;
}

int TreeImpl::onMessageChar(UINT code, USHORT count, USHORT flags)
{
	if (tree_->filterWhenType_) {
		if (code >= 0x20 && code != VK_ESCAPE) {
			if (!(code == VK_BACK && !tree_->filterMode_))
				tree_->setFilterMode(true);
		}
	}
	if (tree_->filterMode_) {
		PostMessageW(tree_->filterEntry_->_window()->handle(), WM_CHAR, code, MAKELPARAM(count, flags));
	}
	return __super::onMessageChar(code, count, flags);
}

int TreeImpl::onMessageGetDlgCode(int keyCode, MSG* msg)
{
	if (!msg)
		return DLGC_WANTMESSAGE;
	else {
		if (tree_->filterMode_) {
			if (keyCode == VK_ESCAPE)
				return DLGC_WANTMESSAGE;
		}
		if (keyCode == VK_RETURN)
			return DLGC_WANTMESSAGE;
	}
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}

LRESULT TreeImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	redrawLock_ = true;
	YASLI_ASSERT(::IsWindow(handle_));
    
	switch(message){
	case WM_HSCROLL:
	case WM_VSCROLL:
		onMessageScroll(message, LOWORD(wparam));
		break;
	case WM_ENABLE:
		YASLI_ASSERT(wparam && "Disabling Window");
		break;
    }
    LRESULT result = __super::onMessage(message, wparam, lparam);
	
	redrawLock_ = false; 
	if(redrawRequest_) 
		onMessagePaint();
	
	return result;
}

}

// vim:ts=4 sw=4:
