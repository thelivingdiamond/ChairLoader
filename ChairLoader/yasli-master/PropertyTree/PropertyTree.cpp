/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/BinArchive.h"
#include "yasli/Pointers.h"
#include "PropertyTree.h"
#include "IDrawContext.h"
#include "Serialization.h"
#include "PropertyTreeModel.h"

#include "yasli/ClassFactory.h"

#include "PropertyOArchive.h"
#include "PropertyIArchive.h"
#include "Unicode.h"

#include <limits.h>
#include "PropertyTreeMenuHandler.h"
#include "IUIFacade.h"
#include "IMenu.h"

#include "MathUtils.h"

#include "PropertyRowObject.h"

using yasli::Serializers;

// ---------------------------------------------------------------------------

TreeConfig::TreeConfig()
: immediateUpdate_(true)
, hideUntranslated_(true)
, valueColumnWidth_(.5f)
, filter_(YASLI_DEFAULT_FILTER)
, compact_(false)
, fullRowMode_(false)
, fullRowContainers_(true)
, showContainerIndices_(true)
, filterWhenType_(true)
, sliderUpdateDelay_(25)
, undoEnabled_(true)
, fullUndo_(false)
, expandLevels_(1)
{
	//QFont font;
	//QFontMetrics fm(font);
	defaultRowHeight_ = 22;//max(17, fm.height() + 6); // to fit at least 16x16 icons
	tabSize_ = defaultRowHeight_;
}

TreeConfig TreeConfig::defaultConfig;

// ---------------------------------------------------------------------------

void PropertyTreeMenuHandler::onMenuFilter()
{
	tree->startFilter("");
}

void PropertyTreeMenuHandler::onMenuFilterByName()
{
	tree->startFilter(filterName.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByValue()
{
	tree->startFilter(filterValue.c_str());
}

void PropertyTreeMenuHandler::onMenuFilterByType()
{
	tree->startFilter(filterType.c_str());
}

void PropertyTreeMenuHandler::onMenuUndo()
{
	tree->model()->undo();
}

void PropertyTreeMenuHandler::onMenuCopy()
{
	tree->copyRow(row);
}

void PropertyTreeMenuHandler::onMenuPaste()
{
	tree->pasteRow(row);
}
// ---------------------------------------------------------------------------

PropertyTree::PropertyTree(IUIFacade* uiFacade)
: attachedPropertyTree_(0)
, ui_(uiFacade)
, autoRevert_(true)
, leftBorder_(0)
, rightBorder_(0)
, cursorX_(0)
, filterMode_(false)

, applyTime_(0)
, revertTime_(0)
, pressPoint_(-1, -1)
, lastStillPosition_(-1, -1)
, pressedRow_(0)
, capturedRow_(0)
, dragCheckMode_(false)
, dragCheckValue_(false)
, archiveContext_(0)
, widgetRow_(0)
, size_(0, 0)
, offset_(0, 0)
, area_(0, 0, 0, 0)
{
	model_.reset(new PropertyTreeModel(this));
	model_->setExpandLevels(expandLevels_);
	model_->setUndoEnabled(undoEnabled_);
	model_->setFullUndo(fullUndo_);
}

PropertyTree::~PropertyTree()
{
	clearMenuHandlers();
	delete ui_;
}

bool PropertyTree::onRowKeyDown(PropertyRow* row, const KeyEvent* ev)
{
	using namespace property_tree;
	PropertyTreeMenuHandler handler;
	handler.row = row;
	handler.tree = this;

	if(row->onKeyDown(this, ev))
		return true;

	switch(ev->key()){
	case KEY_C:
	if (ev->modifiers() == MODIFIER_CONTROL)
		handler.onMenuCopy();
	return true;
	case KEY_V:
	if (ev->modifiers() == MODIFIER_CONTROL)
		handler.onMenuPaste();
	return true;
	case KEY_Z:
	if (ev->modifiers() == MODIFIER_CONTROL)
		if(model()->canUndo()){
			model()->undo();
			return true;
		}
	break;
	case KEY_F2:
	if (ev->modifiers() == 0) {
		if(selectedRow()) {
			selectedRow()->onActivate(this, true);
			selectedRow()->onActivateRelease(this);
		}
	}
	break;
	case KEY_MENU:
	{
		if (ev->modifiers() == 0) {
			std::auto_ptr<property_tree::IMenu> menu(ui()->createMenu());

			if(onContextMenu(row, *menu)){
				Rect rect(row->rect());
				menu->exec(Point(rect.left() + rect.height(), rect.bottom()));
			}
			return true;
		}
		break;
	}
	}

	PropertyRow* focusedRow = model()->focusedRow();
	if(!focusedRow)
		return false;
	PropertyRow* parentRow = focusedRow->nonPulledParent();
	int x = parentRow->horizontalIndex(this, focusedRow);
	int y = model()->root()->verticalIndex(this, parentRow);
	PropertyRow* selectedRow = 0;
	switch(ev->key()){
	case KEY_UP:
		 {
			selectedRow = model()->root()->rowByVerticalIndex(this, --y);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case KEY_DOWN:
	 {
		selectedRow = model()->root()->rowByVerticalIndex(this, ++y);
		if (selectedRow)
			selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
	}
		break;
	case KEY_LEFT:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = --x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && parentRow->expanded()){
			expandRow(parentRow, false);
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_RIGHT:
		selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = ++x);
		if(selectedRow == focusedRow && parentRow->canBeToggled(this) && !parentRow->expanded()){
			expandRow(parentRow, true);
			selectedRow = model()->focusedRow();
		}
		break;
	case KEY_HOME:
		if (ev->modifiers() == MODIFIER_CONTROL) {
			selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MIN);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, 0);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case KEY_END:
		if (ev->modifiers() == MODIFIER_CONTROL) {
			selectedRow = parentRow->rowByHorizontalIndex(this, cursorX_ = INT_MAX);
		}
		else {
			selectedRow = model()->root()->rowByVerticalIndex(this, INT_MAX);
			if (selectedRow)
				selectedRow = selectedRow->rowByHorizontalIndex(this, cursorX_);
		}
		break;
	case KEY_SPACE:
		if (filterWhenType_)
			break;
	case KEY_RETURN:
		if(focusedRow->canBeToggled(this))
			expandRow(focusedRow, !focusedRow->expanded());
		else {
			focusedRow->onActivate(this, false);
			focusedRow->onActivateRelease(this);
		}
		break;
	}
	if(selectedRow){
		onRowSelected(selectedRow, false, false);	
		return true;
	}
	return false;
}

bool PropertyTree::onRowLMBDown(PropertyRow* row, const Rect& rowRect, Point point, bool controlPressed)
{
	pressPoint_ = point;
	row = model()->root()->hit(this, point);
	if(row){
		if(!row->isRoot() && row->plusRect(this).contains(point) && toggleRow(row))
			return true;
		PropertyRow* rowToSelect = row;
		while (rowToSelect && !rowToSelect->isSelectable())
			rowToSelect = rowToSelect->parent();
		if (rowToSelect)
			onRowSelected(rowToSelect, multiSelectable() && controlPressed, true);	
	}

	PropertyTreeModel::UpdateLock lock = model()->lockUpdate();
	row = model()->root()->hit(this, point);
	if(row && !row->isRoot()){
		bool changed = false;
		if (row->widgetRect(this).contains(point)) {
			DragCheckBegin dragCheck = row->onMouseDragCheckBegin();
			if (dragCheck != DRAG_CHECK_IGNORE) {
				dragCheckValue_ = dragCheck == DRAG_CHECK_SET;
				dragCheckMode_ = true;
				changed = row->onMouseDragCheck(this, dragCheckValue_);
			}
		}
		
		if (!dragCheckMode_) {
			bool capture = row->onMouseDown(this, point, changed);
			if(!changed && !widget_.get()){ // FIXME: осмысленный метод для проверки
				if(capture)
					return true;
				else if(row->widgetRect(this).contains(point)){
					if(row->widgetPlacement() != PropertyRow::WIDGET_ICON)
						interruptDrag();
					row->onActivate(this, false);
					return false;
				}
			}
		}
	}
	return false;
}

void PropertyTree::onMouseStill()
{
	if (capturedRow_) {
		PropertyDragEvent e;
		e.tree = this;
		e.pos = ui()->cursorPosition();
		e.start = pressPoint_;

		if (e.pos != lastStillPosition_) {
			capturedRow_->onMouseStill(e);
			lastStillPosition_ = e.pos;
		}
	}
}

void PropertyTree::onRowLMBUp(PropertyRow* _row, const Rect& rowRect, Point point)
{
	onMouseStill();
  SharedPtr<PropertyRow> row(_row);
	row->onMouseUp(this, point);

	if (row->refCount() > 1 && (pressPoint_ - point).manhattanLength() < 2 && row->widgetRect(this).contains(point)) {
		row->onActivateRelease(this);
	}
}

void PropertyTree::onRowRMBDown(PropertyRow* row, const Rect& rowRect, Point point)
{
	SharedPtr<PropertyRow> handle = row;
	PropertyRow* menuRow = 0;

	if (row->isSelectable()){
		menuRow = row;
	}
	else{
		if (row->parent() && row->parent()->isSelectable())
			menuRow = row->parent();
	}

	if (menuRow) {
		onRowSelected(menuRow, false, true);	
		std::auto_ptr<property_tree::IMenu> menu(ui()->createMenu());
		clearMenuHandlers();
		if(onContextMenu(menuRow, *menu))
			menu->exec(point);
	}
}

void PropertyTree::expandParents(PropertyRow* row)
{
	bool hasChanges = false;
	typedef std::vector<PropertyRow*> Parents;
	Parents parents;
	PropertyRow* p = row->nonPulledParent()->parent();
	while(p){
		parents.push_back(p);
		p = p->parent();
	}
	Parents::iterator it;
	for(it = parents.begin(); it != parents.end(); ++it) {
		PropertyRow* row = *it;
		if (!row->expanded() || hasChanges) {
			row->_setExpanded(true);
			hasChanges = true;
		}
	}
	if (hasChanges)
		updateHeights();
}

void PropertyTree::expandAll()
{
	expandChildren(0);
}


void PropertyTree::expandChildren(PropertyRow* root)
{
	if(!root){
		root = model()->root();
		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, true);
		}
		root->setLayoutChanged();
	}
	else
		root->setExpandedRecursive(this, true);

	for (PropertyRow* r = root; r != 0; r = r->parent())
		r->setLayoutChanged();

	updateHeights();
}

void PropertyTree::collapseAll()
{
	collapseChildren(0);
}

void PropertyTree::collapseChildren(PropertyRow* root)
{
	if(!root){
		root = model()->root();

		PropertyRow::iterator it;
		for (PropertyRows::iterator it = root->begin(); it != root->end(); ++it){
			PropertyRow* row = *it;
			row->setExpandedRecursive(this, false);
		}
	}
	else{
		root->setExpandedRecursive(this, false);
		PropertyRow* row = model()->focusedRow();
		while(row){
			if(root == row){
				model()->selectRow(row, true);
				break;
			}
			row = row->parent();
		}
	}

	for (PropertyRow* r = root; r != 0; r = r->parent())
		r->setLayoutChanged();

	updateHeights();
}

void PropertyTree::expandRow(PropertyRow* row, bool expanded, bool updateHeights)
{
	bool hasChanges = false;
	if (row->expanded() != expanded) {
		row->_setExpanded(expanded);
		hasChanges = true;
	}

	for (PropertyRow* r = row; r != 0; r = r->parent())
		r->setLayoutChanged();

    if(!row->expanded()){
		PropertyRow* f = model()->focusedRow();
		while(f){
			if(row == f){
				model()->selectRow(row, true);
				break;
			}
			f = f->parent();
		}
	}

	if (hasChanges && updateHeights)
		this->updateHeights();
}

Point PropertyTree::treeSize() const
{
	return size_ + (compact() ? Point(0,0) : Point(8, 8));
}

void PropertyTree::YASLI_SERIALIZE_METHOD(Archive& ar)
{
	model()->YASLI_SERIALIZE_METHOD(ar, this);

	if(ar.isInput()){
		updateHeights();
		ensureVisible(model()->focusedRow());
		updateAttachedPropertyTree(false);
		updateHeights();
		onSelected();
	}
}

void PropertyTree::ensureVisible(PropertyRow* row, bool update)
{
	if (row == 0)
		return;
	if(row->isRoot())
		return;

	expandParents(row);

	Rect rect = row->rect();
	if(rect.top() < offset_.y()){
		offset_.setY(max(0, rect.top()));
	}
	else if(rect.bottom() > size_.y() + offset_.y()){
		offset_.setY(max(0, rect.bottom() - size_.y()));
	}
	if(update)
		repaint();
}

void PropertyTree::onRowSelected(PropertyRow* row, bool addSelection, bool adjustCursorPos)
{
	if(!row->isRoot())
		model()->selectRow(row, !(addSelection && row->selected() && model()->selection().size() > 1), !addSelection);
	ensureVisible(row);
	if(adjustCursorPos)
		cursorX_ = row->nonPulledParent()->horizontalIndex(this, row);
	updateAttachedPropertyTree(false);
	onSelected();
}

bool PropertyTree::attach(const yasli::Serializers& serializers)
{
	bool changed = false;
	if (attached_.size() != serializers.size())
		changed = true;
	else {
		for (size_t i = 0; i < serializers.size(); ++i) {
			if (attached_[i].serializer() != serializers[i]) {
				changed = true;
				break;
			}
		}
	}

	// We can't perform plain copying here, as it was before:
	//   attached_ = serializers;
	// ...as move forwarder calls copying constructor with non-const argument
	// which invokes second templated constructor of Serializer, which is not what we need.
	if (changed) {
		attached_.assign(serializers.begin(), serializers.end());
		revert();
	}

	return changed;
}

void PropertyTree::attach(const yasli::Serializer& serializer)
{
	if (attached_.size() != 1 || attached_[0].serializer() != serializer) {
		attached_.clear();
		attached_.push_back(yasli::Object(serializer));
		revert();
	}
}

void PropertyTree::attach(const yasli::Object& object)
{
	attached_.clear();
	attached_.push_back(object);

	revert();
}

void PropertyTree::detach()
{
	if(widget_.get())
		widget_.reset();
	attached_.clear();
	model()->root()->clear();
	repaint();
}

int PropertyTree::revertObjects(vector<void*> objectAddresses)
{
	int result = 0;
	for (size_t i = 0; i < objectAddresses.size(); ++i) {
		if (revertObject(objectAddresses[i]))
			++result;
	}
	return result;
}

bool PropertyTree::revertObject(void* objectAddress)
{
	PropertyRow* row = model()->root()->findByAddress(objectAddress);
	if (row && row->isObject()) {
		// TODO:
		// revertObjectRow(row);
		return true;
	}
	return false;
}


void PropertyTree::revert()
{
	interruptDrag();
	widget_.reset();
	capturedRow_ = 0;

	if (!attached_.empty()) {
		//QElapsedTimer timer;
		//timer.start();

		PropertyOArchive oa(model_.get(), model_->root());
		oa.setLastContext(archiveContext_);
		oa.setFilter(filter_);

		Objects::iterator it = attached_.begin();
		onAboutToSerialize(oa);
		(*it)(oa);
		oa.finalize();
		
		PropertyTreeModel model2(this);
		while(++it != attached_.end()){
			PropertyOArchive oa2(&model2, model2.root());
			oa2.setLastContext(archiveContext_);
			yasli::Context treeContext(oa2, this);
			oa2.setFilter(filter_);
			onAboutToSerialize(oa2);
			(*it)(oa2);
			oa2.finalize();
			model_->root()->intersect(model2.root());
		}
		//revertTime_ = int(timer.elapsed());
	}
	else
		model_->clear();

	if (filterMode_) {
		if (model_->root())
			model_->root()->updateLabel(this, 0);
		//resetFilter();
	}
	else {
		updateHeights();
	}

	repaint();
	updateAttachedPropertyTree(true);

	onReverted();
}

void PropertyTree::revertNonInterrupting()
{
	if (!capturedRow_) {
		revert();
	}
}

void PropertyTree::apply(bool continuous)
{
	//QElapsedTimer timer;
	//timer.start();

	if (!attached_.empty()) {
		Objects::iterator it;
		for(it = attached_.begin(); it != attached_.end(); ++it) {
			PropertyIArchive ia(model_.get(), model_->root());
			ia.setLastContext(archiveContext_);
 			yasli::Context treeContext(ia, this);
 			ia.setFilter(filter_);
			onAboutToSerialize(ia);
			(*it)(ia);
		}
	}

	if (continuous)
		onContinuousChange();
	else
		onChanged();

	//applyTime_ = timer.elapsed();
}

bool PropertyTree::spawnWidget(PropertyRow* row, bool ignoreReadOnly)
{
	if(!widget_.get() || widgetRow_ != row){
		interruptDrag();
		setWidget(0, 0);
		property_tree::InplaceWidget* newWidget = 0;
		if ((ignoreReadOnly && row->userReadOnlyRecurse()) || !row->userReadOnly())
			newWidget = row->createWidget(this);
		setWidget(newWidget, row);
		return newWidget != 0;
	}
	return false;
}

bool PropertyTree::activateRow(PropertyRow* row)
{
	interruptDrag();
	return row->onActivate(this, false);
}

void PropertyTree::addMenuHandler(PropertyRowMenuHandler* handler)
{
	menuHandlers_.push_back(handler);
}

void PropertyTree::clearMenuHandlers()
{
	for (size_t i = 0; i < menuHandlers_.size(); ++i)
	{
		PropertyRowMenuHandler* handler = menuHandlers_[i];
		delete handler;
	}
	menuHandlers_.clear();
}

static yasli::string quoteIfNeeded(const char* str)
{
	if (!str)
		return yasli::string();
	if (strchr(str, ' ') != 0) {
		yasli::string result;
		result = "\"";
		result += str;
		result += "\"";
		return result;
	}
	else {
		return yasli::string(str);
	}
}

bool PropertyTree::onContextMenu(PropertyRow* r, IMenu& menu)
{
	SharedPtr<PropertyRow> row(r);
	PropertyTreeMenuHandler* handler = new PropertyTreeMenuHandler();
	addMenuHandler(handler);
	handler->tree = this;
	handler->row = row;

	PropertyRow::iterator it;
	for(it = row->begin(); it != row->end(); ++it){
		PropertyRow* child = *it;
		if(child->isContainer() && child->pulledUp())
			child->onContextMenu(menu, this);
	}
	row->onContextMenu(menu, this);
	if(undoEnabled_){
		if(!menu.isEmpty())
			menu.addSeparator();
		menu.addAction("Undo", "Ctrl+Z", model()->canUndo() ? 0 : MENU_DISABLED, handler, &PropertyTreeMenuHandler::onMenuUndo);
	}
	if(!menu.isEmpty())
		menu.addSeparator();

	menu.addAction("Copy", "Ctrl+C", 0, handler, &PropertyTreeMenuHandler::onMenuCopy);

	if(!row->userReadOnly()){
		menu.addAction("Paste", "Ctrl+V", canBePasted(row) ? 0 : MENU_DISABLED, handler, &PropertyTreeMenuHandler::onMenuPaste);
	}

	menu.addSeparator();

	menu.addAction("Filter...", "Ctrl+F", 0, handler, &PropertyTreeMenuHandler::onMenuFilter);
	IMenu* filter = menu.addMenu("Filter by");
	{
		yasli::string nameFilter = "#";
		nameFilter += quoteIfNeeded(row->labelUndecorated());
		handler->filterName = nameFilter;
		filter->addAction((yasli::string("Name:\t") + nameFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByName);

		yasli::string valueFilter = "=";
		valueFilter += quoteIfNeeded(row->valueAsString().c_str());
		handler->filterValue = valueFilter;
		filter->addAction((yasli::string("Value:\t") + valueFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByValue);

		yasli::string typeFilter = ":";
		typeFilter += quoteIfNeeded(row->typeNameForFilter(this));
		handler->filterType = typeFilter;
		filter->addAction((yasli::string("Type:\t") + typeFilter).c_str(), 0, handler, &PropertyTreeMenuHandler::onMenuFilterByType);
	}

	// menu.addSeparator();
	// menu.addAction(TRANSLATE("Decompose"), row).connect(this, &PropertyTree::onRowMenuDecompose);
	return true;
}

void PropertyTree::onRowMouseMove(PropertyRow* row, const Rect& rowRect, Point point)
{
	PropertyDragEvent e;
	e.tree = this;
	e.pos = point;
	e.start = pressPoint_;
	row->onMouseDrag(e);
	repaint();
}

struct DecomposeProxy
{
	DecomposeProxy(SharedPtr<PropertyRow>& row) : row(row) {}
	
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar)
	{
		ar(row, "row", "Row");
	}

	SharedPtr<PropertyRow>& row;
};

void PropertyTree::onRowMenuDecompose(PropertyRow* row)
{
  // SharedPtr<PropertyRow> clonedRow = row->clone();
  // DecomposeProxy proxy(clonedRow);
  // edit(SStruct(proxy), 0, IMMEDIATE_UPDATE, this);
}

void PropertyTree::onModelUpdated(const PropertyRows& rows, bool needApply)
{
	if(widget_.get()) {
		defocusInplaceEditor();
		widget_.reset();
	}

	if(immediateUpdate_){
		if (needApply)
			apply();

		if(autoRevert_)
			revert();
		else {
			updateHeights();
			updateAttachedPropertyTree(true);
			if(!immediateUpdate_)
				onChanged();
		}
	}
	else {
		repaint();
	}
}

void PropertyTree::onModelPushUndo(PropertyTreeOperator* op, bool* handled)
{
	onPushUndo();
}

void PropertyTree::setWidget(property_tree::InplaceWidget* widget, PropertyRow* widgetRow)
{
	widget_.reset(widget);
	widgetRow_ = widgetRow;
	model()->dismissUpdate();
	if(widget_.get()){
		YASLI_ASSERT(widget_->actualWidget());
		//if(widget_->actualWidget())
		//	((QWidget*)widget_->actualWidget())->setParent(this);
		_arrangeChildren();
		widget_->showPopup();
		repaint();
	}
}

void PropertyTree::setExpandLevels(int levels)
{
	expandLevels_ = levels;
    model()->setExpandLevels(levels);
}

PropertyRow* PropertyTree::selectedRow()
{
    const PropertyTreeModel::Selection &sel = model()->selection();
    if(sel.empty())
        return 0;
    return model()->rowFromPath(sel.front());
}

bool PropertyTree::getSelectedObject(yasli::Object* object)
{
	const PropertyTreeModel::Selection &sel = model()->selection();
	if(sel.empty())
		return 0;
	PropertyRow* row = model()->rowFromPath(sel.front());
	while (row && !row->isObject())
		row = row->parent();
	if (!row)
		return false;

	if (PropertyRowObject* obj = dynamic_cast<PropertyRowObject*>(row)) {
		*object = obj->object();
		return true;
	}
	else {
		return false;
	}
}

bool PropertyTree::selectByAddress(const void* addr, bool keepSelectionIfChildSelected)
{
	return selectByAddresses(vector<const void*>(1, addr), keepSelectionIfChildSelected);
}

bool PropertyTree::selectByAddresses(const vector<const void*>& addresses, bool keepSelectionIfChildSelected)
{
	if (model()->root()) {
		TreeSelection sel;

		bool keepSelection = false;
		for (size_t i = 0; i < addresses.size(); ++i) {
			const void* addr = addresses[i];
			PropertyRow* row = model()->root()->findByAddress(addr);

			if (keepSelectionIfChildSelected && row && !model()->selection().empty()) {
				keepSelection = true;
				TreeSelection::const_iterator it;
				for(it = model()->selection().begin(); it != model()->selection().end(); ++it){
					PropertyRow* selectedRow = model()->rowFromPath(*it);
					if (!selectedRow)
						continue;
					if (!selectedRow->isChildOf(row)){
						keepSelection = false;
						break;
					}
				}
			}

			if (!keepSelection) {
				if(row) {
					sel.push_back(model()->pathFromRow(row));
					ensureVisible(row);
				}
			}
		}

		if (model()->selection() != sel) {
			model()->setSelection(sel);
			updateAttachedPropertyTree(true); 
			repaint();
			return true;
		}
	}
	return false;
}

void PropertyTree::setUndoEnabled(bool enabled, bool full)
{
	undoEnabled_ = enabled; fullUndo_ = full;
    model()->setUndoEnabled(enabled);
    model()->setFullUndo(full);
}

void PropertyTree::attachPropertyTree(PropertyTree* propertyTree) 
{ 
	attachedPropertyTree_ = propertyTree; 
	updateAttachedPropertyTree(true); 
}

void PropertyTree::getSelectionSerializers(yasli::Serializers* serializers)
{
	TreeSelection::const_iterator i;
	for(i = model()->selection().begin(); i != model()->selection().end(); ++i){
		PropertyRow* row = model()->rowFromPath(*i);
		if (!row)
			continue;


		while(row && ((row->pulledUp() || row->pulledBefore()) || row->isLeaf())) {
			row = row->parent();
		}
		Serializer ser = row->serializer();

		if (ser)
			serializers->push_back(ser);
	}
}

void PropertyTree::updateAttachedPropertyTree(bool revert)
{
	if(attachedPropertyTree_) {
 		Serializers serializers;
 		getSelectionSerializers(&serializers);
 		if (!attachedPropertyTree_->attach(serializers) && revert)
			attachedPropertyTree_->revert();
 	}
}

void toLowerUtf8(char* str)
{
#ifdef WW_DISABLE_UTF8
	for(;*str; ++str)
		*str = tolower(*str);
#else
	std::wstring ws = toWideChar(str);
	_wcslwr_s((wchar_t*)ws.c_str(), ws.size() + 1);
	std::string sl = fromWideChar(ws.c_str());
	strcpy(str, sl.c_str());
#endif
}

size_t countCharsUtf8(const char* str, const char* subStr)
{
	YASLI_ASSERT(str <= subStr);
#ifdef WW_DISABLE_UTF8
	return subStr - str;
#else
	size_t count = 0;
	for(;str != subStr; ++str)
		if((unsigned char)*str >> 7 == 0 || (unsigned char)*str >> 6 == 3)
			++count;
	return count;
#endif
}

void PropertyTree::RowFilter::parse(const char* filter)
{
	for (int i = 0; i < NUM_TYPES; ++i) {
		start[i].clear();
		substrings[i].clear();
		tillEnd[i] = false;
	}

	YASLI_ESCAPE(filter != 0, return);

	vector<char> filterBuf(filter, filter + strlen(filter) + 1);
	toLowerUtf8(&filterBuf[0]);
	const char* str = &filterBuf[0];

	Type type = NAME_VALUE;
	while (true)
	{
		bool fromStart = false;
		while (*str == '^') {
			fromStart = true;
			++str;
		}

		const char* tokenStart = str;
		
		if (*str == '\"')
		{
			++str;
			while(*str != '\0' && *str != '\"')
				++str;
		}
		else
		{
			while (*str != '\0' && *str != ' ' && *str != '*' && *str != '=' && *str != ':' && *str != '#')
					++str;
		}
		if (str != tokenStart) {
			if (*tokenStart == '\"' && *str == '\"') {
				start[type].assign(tokenStart + 1, str);
				tillEnd[type] = true;
				++str;
			}
			else
			{
				if (fromStart)
					start[type].assign(tokenStart, str);
				else
					substrings[type].push_back(yasli::string(tokenStart, str));
			}
		}
		while (*str == ' ')
			++str;
		if (*str == '#') {
			type = NAME;
			++str;
		}
		else if (*str == '=') {
			type = VALUE;
			++str;
		}
		else if(*str == ':') {
			type = TYPE;
			++str;
		}
		else if (*str == '\0')
			break;
	}
}

bool PropertyTree::RowFilter::match(const char* textOriginal, Type type, size_t* matchStart, size_t* matchEnd) const
{
	YASLI_ESCAPE(textOriginal, return false);

	char* text;
	{
		size_t textLen = strlen(textOriginal);
        text = (char*)alloca((textLen + 1));
		memcpy(text, textOriginal, (textLen + 1));
		toLowerUtf8(text);
	}
	
	const yasli::string &start = this->start[type];
	if (tillEnd[type]){
		if (start == text) {
			if (matchStart)
				*matchStart = 0;
			if (matchEnd)
				*matchEnd = start.size();
			return true;
		}
		else
			return false;
	}

	const vector<yasli::string> &substrings = this->substrings[type];

	const char* startPos = text;

	if (matchStart)
		*matchStart = 0;
	if (matchEnd)
		*matchEnd = 0;
	if (!start.empty()) {
		if (strncmp(text, start.c_str(), start.size()) != 0){
            //_freea(text);
			return false;
		}
		if (matchEnd)
			*matchEnd = start.size();
		startPos += start.size();
	}

	size_t numSubstrings = substrings.size();
	for (size_t i = 0; i < numSubstrings; ++i) {
		const char* substr = strstr(startPos, substrings[i].c_str());
		if (!substr){
			return false;
		}
		startPos += substrings[i].size();
		if (matchStart && i == 0 && start.empty()) {
			*matchStart = countCharsUtf8(text, substr);
		}
		if (matchEnd)
			*matchEnd = countCharsUtf8(text, substr + substrings[i].size());
	}
	return true;
}

PropertyRow* PropertyTree::rowByPoint(const Point& pt)
{
	if (!model_->root())
		return 0;
	if (!area_.contains(pt))
		return 0;
  return model_->root()->hit(this, pointToRootSpace(pt));
}

Point PropertyTree::pointToRootSpace(const Point& point) const
{
	return Point(point.x() + offset_.x(), point.y() + offset_.y());
}

bool PropertyTree::toggleRow(PropertyRow* row)
{
	if(!row->canBeToggled(this))
		return false;
	expandRow(row, !row->expanded());
	updateHeights();
	return true;
}

bool PropertyTree::_isCapturedRow(const PropertyRow* row) const
{
	return capturedRow_ == row;
}

void PropertyTree::setArchiveContext(yasli::Context* lastContext)
{
	archiveContext_ = lastContext;
}

PropertyTree::PropertyTree(const PropertyTree&)
{
}


PropertyTree& PropertyTree::operator=(const PropertyTree&)
{
	return *this;
}

Point PropertyTree::_toWidget(Point point) const
{
	return Point(point.x() - offset_.x(), point.y() - offset_.y());
}

void PropertyTree::_cancelWidget()
{
	defocusInplaceEditor();
	widget_.reset();
}

// vim:ts=4 sw=4:
