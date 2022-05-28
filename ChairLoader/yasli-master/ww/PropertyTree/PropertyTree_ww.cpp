/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "ww/PropertyTree.h"
#include "ww/PropertyTree/wwDrawContext.h"
#include "ww/PropertyTree/TreeImpl.h"
#include "PropertyTree/Serialization.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/PropertyRowContainer.h"
#include "PropertyTree/PropertyRowPointer.h"
#include "PropertyTree/Unicode.h"
#include "PropertyTree/PropertyOArchive.h"
#include "PropertyTree/PropertyIArchive.h"
#include "ww/Window.h"
#include "ww/Color.h"

#include "yasli/ClassFactory.h"

#include "ww/Clipboard.h"

#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h" 
#include "ww/Win32/Handle.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Entry.h"
#include <crtdbg.h>

#include "ww/PropertyEditor.h"
#include "wwUIFacade.h"
#include "wwDrawContext.h"
#include "gdiplusUtils.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

namespace ww{

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





class FilterEntry : public Entry
{
public:
	FilterEntry(PropertyTree* tree)
	: tree_(tree)
	{
		setSwallowArrows(true);
		setSwallowReturn(true);
		setSwallowEscape(true);
	}
protected:
	bool onKeyPress(const KeyPress& key)
	{
		if (key.key == KEY_UP ||
			key.key == KEY_DOWN ||
			key.key == KEY_ESCAPE ||
			key.key == KEY_RETURN)
		{
			SetFocus(tree_->impl()->handle());
			PostMessageW(tree_->impl()->handle(), WM_KEYDOWN, key.key, 0);
			return true;
		}
		if (key.key == KEY_BACK && text()[0] == '\0')
		{
			tree_->setFilterMode(false);
		}
		return false;
	}
private:
	PropertyTree* tree_;
};

// ---------------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
PropertyTree::PropertyTree(int border)
: ::PropertyTree(new property_tree::wwUIFacade(this))
, _ContainerWithWindow(new TreeImpl(this), border)
{
	dragController_.reset(new DragController(impl()));

	DrawingCache::get()->initialize();

	(TreeConfig&)*this = defaultConfig;

	HDC dc = GetDC(impl()->handle());
	graphics_ = new Gdiplus::Graphics(dc);

	_setMinimalSize(0, 0);

	filterEntry_ = new FilterEntry(this);
	filterEntry_->_setParent(this);
	filterEntry_->signalChanged().connect(this, &PropertyTree::onFilterChanged);

}
#pragma warning(pop)

struct DrawVisitor
{
	DrawVisitor(Gdiplus::Graphics* graphics, const property_tree::Rect& area, int scrollOffset, bool selectionPass)
		: area_(area)
		, graphics_(graphics)
		, scrollOffset_(scrollOffset)
		, lastParent_(0)
		, selectionPass_(selectionPass)
	{}

	ScanResult operator()(PropertyRow* row, ::PropertyTree* _tree, int index)
	{
		if(row->visible(_tree) && (row->parent()->expanded() && !lastParent_ || row->pulledUp())){
			if(row->rect().top() > scrollOffset_ + area_.height())
				lastParent_ = row->parent();

			if(row->rect().bottom() > scrollOffset_)
			{
				wwDrawContext context((ww::PropertyTree*)_tree, graphics_);
				row->drawRow(context, _tree, index, selectionPass_);
			}

			return SCAN_CHILDREN_SIBLINGS;
		}
		else
			return SCAN_SIBLINGS;
	}

protected:
	Gdiplus::Graphics* graphics_;
	property_tree::Rect area_;
	int scrollOffset_;
	PropertyRow* lastParent_;
	bool selectionPass_;
};


PropertyTree::~PropertyTree()
{
	ReleaseDC(impl()->handle(), graphics_->GetHDC());
	delete graphics_;
	graphics_ = 0;

	DrawingCache::get()->finalize();
}

void PropertyTree::update()
{
	::InvalidateRect(impl()->handle(), 0, FALSE);
}

TreeImpl* PropertyTree::impl() const
{
	return static_cast<TreeImpl*>(_window());
}


void PropertyTree::interruptDrag()
{
	dragController_->interrupt();
}

void PropertyTree::updateHeights()
{
	bool force = false;
	{
		DebugTimer("updateHeights");

		Win32::Rect clientRect;
		::GetClientRect(impl()->handle(), &clientRect);

		area_ = property_tree::Rect(clientRect.left + 1, clientRect.top + 1, clientRect.width() - 2, clientRect.height() - 2);

		int filterAreaHeight = 0;
		if (filterMode_)
			filterAreaHeight = filterEntry_->_minimalSize().y + 4;

		updateScrollBar();

		model()->root()->updateLabel(this, 0);
		int lb = compact_ ? 0 : 4;
		int rb = area_.w - lb*2;
		force = force || lb != leftBorder_ || rb != rightBorder_;
		leftBorder_ = lb;
		rightBorder_ = rb;
		model()->root()->calculateMinimalSize(this, leftBorder_, force, 0, 0);

		size_.y_ = area_.top() + filterAreaHeight;
		model()->root()->adjustVerticalPosition(this, size_.y_);
	}
	updateScrollBar();
	update();
}


void PropertyTree::_setFocus()
{
	_ContainerWithWindow::_setFocus();
}


void PropertyTree::copyRow(PropertyRow* row)
{
	Clipboard clipboard(this, model()->constStrings(), model());
    clipboard.copy(row);
}

void PropertyTree::pasteRow(PropertyRow* row)
{
	if(!canBePasted(row))
		return;
	PropertyRow* parent = row->parent() ? row->parent() : model()->root();
    model()->rowAboutToBeChanged(row);
	Clipboard clipboard(this, model()->constStrings(), model());
	if(clipboard.paste(row)){
		parent->setLabelChanged();
		parent->setLabelChangedToChildren();
		model()->rowChanged(parent);
	}
	else
		YASLI_ASSERT(0 && "Unable to paste element!"); 
}


bool PropertyTree::canBePasted(PropertyRow* destination)
{
	if(destination->userReadOnly())
		return false;
	Clipboard clipboard(this, model()->constStrings(), model());
	return clipboard.paste(destination, true);
}

bool PropertyTree::canBePasted(const char* destinationType)
{
	Clipboard clipboard(this, model()->constStrings(), model());
	return clipboard.canBePastedOn(destinationType);
}

bool PropertyTree::hasFocusOrInplaceHasFocus() const
{
	return hasFocus(); // TODO
}
bool PropertyTree::hasFocus() const
{
	HWND focusedWindow = GetFocus();
	return focusedWindow == impl()->handle() || IsChild(impl()->handle(), focusedWindow);
}

void PropertyTree::setFilterMode(bool inFilterMode)
{
    bool changed = filterMode_ != inFilterMode;
    filterMode_ = inFilterMode;
    
	if (filterMode_)
	{
        filterEntry_->show();
		filterEntry_->setFocus();
		filterEntry_->setSelection(ww::EntrySelection(0, -1));
	}
    else
        filterEntry_->hide();

    if (changed)
    {
        onFilterChanged();
		updateHeights();
    }
}

void PropertyTree::startFilter(const char* filter)
{
	setFilterMode(true);
	filterEntry_->setText(filter);
	onFilterChanged();
}


void PropertyTree::visitChildren(WidgetVisitor& visitor) const
{
	if(widget_.get())
		visitor(*((ww::Widget*)widget_->actualWidget()));
}

void PropertyTree::_arrangeChildren()
{
	if(widget_.get()){
		if(widgetRow_->visible(this)){
			Widget* w = (Widget*)widget_->actualWidget();
			YASLI_ASSERT(w);
			if(w){
				property_tree::Rect rect = widgetRow_->widgetRect(this);
				ww::Rect r(rect.left() - offset_.x() + area_.left(),
					rect.top() - offset_.y() + area_.top(),
					rect.right() - offset_.x(),
					rect.bottom() - offset_.y()
					);
				w->_setPosition(r);
				if(!w->isVisible()){
					w->show();
					w->setFocus();
				}
			}
			else{
				//YASLI_ASSERT(w);
			}
		}
		else{
			widget_.reset();
		}
	}

    if (filterEntry_) {
        Vect2 size = _position().size();
        const int padding = 2;
		Rect pos(60, padding, size.x - padding - GetSystemMetrics(SM_CXVSCROLL), filterEntry_->_minimalSize().y + padding);
        filterEntry_->_setPosition(pos);
    }
}

Vect2 PropertyTree::_toScreen(Vect2 point) const
{
    POINT pt = { point.x - offset_.x() + area_.left(), 
		         point.y - offset_.y() + area_.top() };
    ClientToScreen(impl()->handle(), &pt);
    return Vect2(pt.x, pt.y);
}

void PropertyTree::attachPropertyTree(::PropertyTree* propertyTree) 
{ 
	if(attachedPropertyTree_)
		((PropertyTree*)attachedPropertyTree_)->signalChanged().disconnect((ww::Widget*)this);
	__super::attachPropertyTree(propertyTree);
	//((PropertyTree*)attachedPropertyTree_)->signalChanged().connect(this, &PropertyTree::onAttachedTreeChanged);
}

struct FilterVisitor
{
	const PropertyTree::RowFilter& filter_;

	FilterVisitor(const PropertyTree::RowFilter& filter) 
    : filter_(filter)
    {
    }

	static void markChildrenAsBelonging(PropertyRow* row, bool belongs)
	{
		int count = int(row->count());
		for (int i = 0; i < count; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			child->setBelongsToFilteredRow(belongs);

			markChildrenAsBelonging(child, belongs);
		}
	}
	
	static bool hasMatchingChildren(PropertyRow* row)
	{
		int numChildren = (int)row->count();
		for (int i = 0; i < numChildren; ++i)
		{
			PropertyRow* child = row->childByIndex(i);
			if (!child)
				continue;
			if (child->matchFilter())
				return true;
			if (hasMatchingChildren(child))
				return true;
		}
		return false;
	}

	ScanResult operator()(PropertyRow* row, ::PropertyTree* _tree)
	{
		PropertyTree* tree = (PropertyTree*)_tree;
		string label = row->labelUndecorated();
		string value = row->valueAsString();
		bool matchFilter = filter_.match(label.c_str(), filter_.NAME_VALUE, 0, 0) || filter_.match(value.c_str(), filter_.NAME_VALUE, 0, 0);
		if (matchFilter && filter_.typeRelevant(filter_.NAME))
			matchFilter = filter_.match(label.c_str(), filter_.NAME, 0, 0);
		if (matchFilter && filter_.typeRelevant(filter_.VALUE))
			matchFilter = filter_.match(value.c_str(), filter_.VALUE, 0, 0);
		if (matchFilter && filter_.typeRelevant(filter_.TYPE))
			matchFilter = filter_.match(row->typeNameForFilter(tree), filter_.TYPE, 0, 0);						   
		
		int numChildren = int(row->count());
		if (matchFilter) {
			if (row->pulledBefore() || row->pulledUp()) {
				// treat pulled rows as part of parent
				PropertyRow* parent = row->parent();
				parent->setMatchFilter(true);
				markChildrenAsBelonging(parent, true);
				parent->setBelongsToFilteredRow(false);
			}
			else {
				markChildrenAsBelonging(row, true);
				row->setBelongsToFilteredRow(false);
				row->setLayoutChanged();
				row->setLabelChanged();
			}
		}
		else {
			bool belongs = hasMatchingChildren(row);
			row->setBelongsToFilteredRow(belongs);
			if (belongs) {
				tree->expandRow(row, true, false);
				for (int i = 0; i < numChildren; ++i) {
					PropertyRow* child = row->childByIndex(i);
					if (child->pulledUp())
						child->setBelongsToFilteredRow(true);
				}
			}
			else {
				row->_setExpanded(false);
				row->setLayoutChanged();
			}
		}

		row->setMatchFilter(matchFilter);
		return SCAN_CHILDREN_SIBLINGS;
	}

protected:
	string labelStart_;
};

void PropertyTree::onFilterChanged()
{
	const char* filterStr = filterMode_ ? filterEntry_->text() : "";
	rowFilter_.parse(filterStr);
	FilterVisitor visitor(rowFilter_);
	model()->root()->scanChildrenBottomUp(visitor, this);
	updateHeights();
}

void PropertyTree::drawFilteredString(Gdiplus::Graphics* gr, const char* text, RowFilter::Type type, Gdiplus::Font* font, const Rect& rect, const Color& textColor, bool pathEllipsis, bool center) const
{
	if (rect.width() <= 0 || rect.height() <= 0)
		return;
	int textLen = (int)strlen(text);

	Gdiplus::StringFormat format;
	format.SetAlignment(center ? Gdiplus::StringAlignmentCenter : Gdiplus::StringAlignmentNear);
	format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	format.SetTrimming(pathEllipsis ? Gdiplus::StringTrimmingEllipsisPath : Gdiplus::StringTrimmingEllipsisCharacter);
	format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

	Gdiplus::RectF textRect(gdiplusRectF(rect));
	wstring wtext(toWideChar(text));
	if (filterMode_) {
		size_t hiStart = 0;
		size_t hiEnd = 0;
		bool matched = rowFilter_.match(text, type, &hiStart, &hiEnd) && hiStart != hiEnd;
		if (!matched && (type == RowFilter::NAME || type == RowFilter::VALUE))
			matched = rowFilter_.match(text, RowFilter::NAME_VALUE, &hiStart, &hiEnd);
        if (matched && hiStart != hiEnd) {
			Gdiplus::RectF boxFull;
			Gdiplus::RectF boxStart;
			Gdiplus::RectF boxEnd;
			
			gr->MeasureString(wtext.c_str(), textLen, font, textRect, &format, &boxFull, 0, 0);
			
			if (hiStart > 0)
				gr->MeasureString(wtext.c_str(), (int)hiStart, font, textRect, &format, &boxStart, 0, 0);
			else {
				gr->MeasureString(wtext.c_str(), textLen, font, textRect, &format, &boxStart, 0, 0);
				boxStart.Width = 0.0;
			}
			gr->MeasureString(wtext.c_str(), (int)hiEnd, font, textRect, &format, &boxEnd, 0, 0);

			ww::Color highlightColor, highlightBorderColor;
			{
				highlightColor.setGDI(GetSysColor(COLOR_HIGHLIGHT));
				float h, s, v;
				highlightColor.toHSV(h, s, v);
				h -= 175.0f;
				if (h < 0.0f)
					h += 360.0f;
				highlightColor.setHSV(h, min(1.0f, s * 1.33f), 1.0f, 255);
				highlightBorderColor.setHSV(h, s * 0.5f, 1.0f, 255);
			}

			Gdiplus::SolidBrush br(Gdiplus::Color(highlightColor.argb()));
			int left = int(boxFull.X + boxStart.Width) - 1;
			int top = int(boxFull.Y);
			int right = int(boxFull.X + boxEnd.Width);
			int bottom = int(boxFull.Y + boxEnd.Height);
			Gdiplus::Rect highlightRect(left, top, right - left, bottom - top);

			fillRoundRectangle(gr, &br, highlightRect, Gdiplus::Color(highlightBorderColor.argb()) /*Gdiplus::Color(255, 255, 128)*/, 1);
		}
	}

	Gdiplus::SolidBrush brush(textColor.argb());
	gr->DrawString(wtext.c_str(), wtext.size(), font, textRect, &format, &brush); 
}

void PropertyTree::_drawRowLabel(Gdiplus::Graphics* gr, const char* text, Gdiplus::Font* font, const Rect& rect, const Color& textColor) const
{
	drawFilteredString(gr, text, RowFilter::NAME, font, rect, textColor, false, false);
}

void PropertyTree::_drawRowValue(Gdiplus::Graphics* gr, const char* text, Gdiplus::Font* font, const Rect& rect, const Color& textColor, bool pathEllipsis, bool center) const
{
	drawFilteredString(gr, text, RowFilter::VALUE, font, rect, textColor, pathEllipsis, center);
}

void PropertyTree::onPaint(HDC dc)
{
	using namespace Gdiplus;
	DebugTimer timer("redraw");

	RECT clientRect = { area_.left(), area_.top(), area_.right(), area_.bottom() };
	::GetClientRect(impl()->handle(), &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	Graphics gr(dc);
	{
		gr.FillRectangle(&SolidBrush(gdiplusSysColor(COLOR_BTNFACE)), clientRect.left,
			clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	}

	if (filterMode_)
	{
		Win32::AutoSelector font(dc, Win32::defaultBoldFont());

		SetBkMode(dc, TRANSPARENT);
		const wchar_t filterStr[] = L"Filter:";
		Vect2 size = Win32::calculateTextSize(impl()->handle(), Win32::defaultBoldFont(), filterStr);
		int right = filterEntry_->_position().left();
		ExtTextOutW(dc, right - size.x - 6, 6, 0, 0, filterStr, ARRAY_LEN(filterStr) - 1, 0);
	}

	::IntersectClipRect(dc, area_.left(), area_.top(), area_.right(), area_.bottom());

	//OffsetViewportOrgEx(dc, -offset_.x(), -offset_.y(), 0);
	if (dragController_->captured())
		dragController_->drawUnder(dc);
	//OffsetViewportOrgEx(dc, offset_.x(), offset_.y(), 0);


	OffsetViewportOrgEx(dc, -offset_.x(), -offset_.y(), 0);
	if (model()->root()) {
		Gdiplus::Graphics gr(dc);
		model()->root()->scanChildren(DrawVisitor(&gr, area_, offset_.y(), true), this);
		model()->root()->scanChildren(DrawVisitor(&gr, area_, offset_.y(), false), this);
	}
	OffsetViewportOrgEx(dc, offset_.x(), offset_.y(), 0);

	::SelectClipRgn(dc, NULL);

	if(size_.y() > clientHeight)
	{
		using namespace Gdiplus;
		using Gdiplus::Rect;
		using Gdiplus::Color;
		Graphics gr(dc);
		const int shadowHeight = 10;
		Color color1(0, 0, 0, 0);
		Color color2(96, 0, 0, 0);

		Rect upperRect(area_.left() - 1, area_.top() - 1, area_.width() + 2, shadowHeight + 1);
		LinearGradientBrush upperBrush(upperRect, color2, color1, LinearGradientModeVertical);
		upperRect.Y += 1;
		upperRect.Height -= 2;
		gr.FillRectangle(&upperBrush, upperRect);

		Rect lowerRect(area_.left(), area_.bottom() - shadowHeight / 2 - 1, area_.width(), shadowHeight / 2 + 2);
		LinearGradientBrush lowerBrush(lowerRect, color1, color2, LinearGradientModeVertical);
		lowerRect.Y += 1;
		lowerRect.Height -= 2;
		gr.FillRectangle(&lowerBrush, lowerRect);

		SolidBrush brush(gdiplusSysColor(COLOR_3DDKSHADOW));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.top(), 1, area_.height()));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.top(), area_.width(), 1));
		gr.FillRectangle(&brush, Rect(clientRect.right - 1, area_.top(), 1, area_.height()));
		gr.FillRectangle(&brush, Rect(clientRect.left, area_.bottom() - 1, area_.width(), 1));
	}

	if(dragController_->captured()){
		//OffsetViewportOrgEx(dc, -offset_.x, -offset_.y, 0);
		dragController_->drawOver(dc);
		//OffsetViewportOrgEx(dc, offset_.x, offset_.y, 0);
	}
	else{
		if(model()->focusedRow() != 0 && model()->focusedRow()->isRoot() && hasFocus()){
			clientRect.left += 2; clientRect.top += 2;
			clientRect.right -= 2; clientRect.bottom -= 2;
			DrawFocusRect(dc, &clientRect);
		}
	}
}


void PropertyTree::onLButtonDown(int button, int x, int y)
{
	::SetFocus(impl()->handle());
	PropertyRow* row = rowByPoint(property_tree::Point(x, y));
	if(row && !row->isSelectable())
		row = row->parent();
	if (row){
		if (onRowLMBDown(row, row->rect(), pointToRootSpace(property_tree::Point(x, y)), (GetKeyState(VK_CONTROL) >> 15) != 0)) {
			SetCapture(impl()->handle());
			capturedRow_ = row;
		}
		else if (!dragCheckMode_){
			row = rowByPoint(property_tree::Point(x, y));
			PropertyRow* draggedRow = row;
			while (draggedRow && (!draggedRow->isSelectable() || draggedRow->pulledUp() || draggedRow->pulledBefore()))
				draggedRow = draggedRow->parent();
			if (draggedRow && !draggedRow->userReadOnly() && !widget_.get()){
				POINT cursorPos = { x, y };
				ClientToScreen(impl()->handle(), &cursorPos);
				dragController_->beginDrag(row, draggedRow, cursorPos);
			}
		}
		else {
			SetCapture(impl()->handle());
		}
	}
	else
		repaint();
}

void PropertyTree::onLButtonUp(int button, int x, int y)
{
	HWND focusedWindow = GetFocus();
	if(focusedWindow){
		YASLI_ASSERT(::IsWindow(focusedWindow));
		Win32::Window32 wnd(focusedWindow);
		if(Win32::Window32* window = reinterpret_cast<Win32::Window32*>(wnd.getUserDataLongPtr())){
			YASLI_ASSERT(window->handle() == focusedWindow);
		}
	}

	if(GetCapture() == impl()->handle())
		ReleaseCapture();

	if(dragController_->captured()){
		POINT pos;
		GetCursorPos(&pos);
		dragController_->drop(pos);
		RedrawWindow(impl()->handle(), 0, 0, RDW_INVALIDATE);
	}
	if (dragCheckMode_) {
		dragCheckMode_ = false;
	}
	else {
		Vect2 point(x, y);
		PropertyRow* row = rowByPoint(property_tree::Point(point.x, point.y));
		if(capturedRow_){

			onRowLMBUp(capturedRow_, capturedRow_->rect(), pointToRootSpace(Point(x, y)));
			capturedRow_ = 0;
			repaint();
		}
	}
}

void PropertyTree::onRButtonDown(int button, int x, int y)
{
	::SetFocus(impl()->handle());

	property_tree::Point point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		model()->setFocusedRow(row);
		RedrawWindow(impl()->handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

		onRowRMBDown(row, row->rect(), pointToRootSpace(point));
	}
	else{
		Win32::Rect rect;
		GetClientRect(impl()->handle(), &rect);
		if (model()->root())
			onRowRMBDown(model()->root(), property_tree::Rect(rect.left, rect.top, rect.width(), rect.height()), pointToRootSpace(point));
	}
}

bool PropertyTree::updateScrollBar()
{
	int pageSize = area_.height();
	offset_.x_ = max(0, min(offset_.x(), max(0, size_.x() - area_.right() - 1)));
	offset_.y_ = max(0, min(offset_.y(), max(0, size_.y() - pageSize)));

	SCROLLINFO scrollInfo;

	memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = size_.y();
	scrollInfo.nPos = offset_.y();
	scrollInfo.nPage = pageSize;
	SetScrollInfo(impl()->handle(), SB_VERT, &scrollInfo, TRUE);

	memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = size_.x();
	scrollInfo.nPos = offset_.x();
	scrollInfo.nPage = area_.right() + 1;
	SetScrollInfo(impl()->handle(), SB_HORZ, &scrollInfo, TRUE);
	return true;
}


void PropertyTree::onScroll(int y)
{
	offset_.setY(y);
}


void PropertyTree::defocusInplaceEditor()
{
	SetFocus(impl()->handle());
}

void PropertyTree::resetFilter()
{
	if (filterEntry_)
		filterEntry_->setText("");
	onFilterChanged();
}


void PropertyTree::onLButtonDoubleClick(int x, int y)
{
	::SetFocus(impl()->handle());

	Point point(x, y);
	PropertyRow* row = rowByPoint(point);
	if(row){
		YASLI_ASSERT(row->refCount() > 0);
		if(row->widgetRect(this).contains(pointToRootSpace(point))){
			if(!row->onActivate(this, true) &&
				!row->onActivateRelease(this))
				toggleRow(row);	
		}
		else if(!toggleRow(row)) {
			row->onActivate(this, false);
			row->onActivateRelease(this);
		}
	}
}


void PropertyTree::onMouseMove(int button, int x, int y)
{
	if(dragController_->captured() && !(button & MK_LBUTTON))
		dragController_->interrupt();
	if (dragController_->captured()){
		POINT pos;
		GetCursorPos(&pos);
		if (dragController_->dragOn(pos) && GetCapture() != impl()->handle())
			SetCapture(impl()->handle());
		RedrawWindow(impl()->handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else{
		property_tree::Point point(x, y);
		PropertyRow* row = rowByPoint(pointToRootSpace(point));
		if (row && dragCheckMode_ && row->widgetRect(this).contains(pointToRootSpace(point))) {
			row->onMouseDragCheck(this, dragCheckValue_);
		}
		else if(capturedRow_){
			onRowMouseMove(capturedRow_, property_tree::Rect(), point);
		}
	}
}

void PropertyTree::onMouseWheel(int delta)
{
	offset_.y_ = offset_.y() - delta;
	updateScrollBar();
	_arrangeChildren();
	RedrawWindow(impl()->handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}

bool PropertyTree::_isDragged(const PropertyRow* row) const
{
	if (!dragController_->isDragging())
		return false;
	if (dragController_->draggedRow() == row)
		return true;
	return false;
}

void PropertyTree::onAttachedTreeChanged()
{
	revert();
}

void PropertyTree::serialize(Archive& ar)
{
	::PropertyTree::serialize(ar);
}

FORCE_SEGMENT(PropertyRowBitVector)
FORCE_SEGMENT(PropertyRowDecorators)
FORCE_SEGMENT(PropertyRowFileSelector)
FORCE_SEGMENT(PropertyRowHotkey)
FORCE_SEGMENT(PropertyRowIconWW)
FORCE_SEGMENT(PropertyRowColorWW)

}


// vim:ts=4 sw=4:
