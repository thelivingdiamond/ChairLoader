/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowContainer.h"
#include "IDrawContext.h"
#include "yasli/ClassFactory.h"
#include "Unicode.h"
#include "Serialization.h"
#include "IUIFacade.h"

#include "yasli/BinArchive.h"

#include "IMenu.h"
#include "MathUtils.h"

#if 0
# define DEBUG_TRACE(fmt, ...) printf(fmt "\n", __VA_ARGS__)
# define DEBUG_TRACE_ROW(fmt, ...) for(PropertyRow* zzzz = this; zzzz; zzzz = zzzz->parent()) printf(" "); printf(fmt "\n", __VA_ARGS__)
#else
# define DEBUG_TRACE(...)
# define DEBUG_TRACE_ROW(...)
#endif
	

inline unsigned calcHash(const char* str, unsigned hash = 5381)
{
	while(*str)
		hash = hash*33 + (unsigned char)*str++;
	return hash;
}

template<class T>
inline unsigned calcHash(const T& t, unsigned hash = 5381)
{
	for (int i = 0; i < sizeof(T); i++)
		hash = hash * 33 + ((unsigned char*)&t)[i];
	return hash;
}

// ---------------------------------------------------------------------------

ConstStringList* PropertyRow::constStrings_ = 0;

PropertyRow::PropertyRow()
{
	parent_ = 0;

	expanded_ = false;
	selected_ = false;
	visible_ = true;
	labelUndecorated_ = 0;
    belongsToFilteredRow_ = false;
    matchFilter_ = true;
	
	pos_ =  Point(0, 0);
	size_ = Point(-1, -1);
	plusSize_ = 0;
	textPos_ = 0;
	textSizeInitial_ = 0;
	textHash_ = 0;
	textSize_ = 0;
	widgetPos_ = 0;
    widgetSize_ = 0;
	userWidgetSize_ = -1;
	
	name_ = "";
	typeName_ = "";
	
	pulledUp_ = false;
	pulledBefore_ = false;
	hasPulled_ = false;
	userReadOnly_ = false;
	userReadOnlyRecurse_ = false;
	userFullRow_ = false;
	multiValue_ = false;
	userHideChildren_ = false;
	updated_ = false;
	
	label_ = "";
	labelChanged_ = true;
	layoutChanged_ = true;
}

PropertyRow::~PropertyRow()
{
	size_t count = children_.size();
	for (size_t i = 0; i < count; ++i)
		if (children_[i]->parent() == this)
			children_[i]->setParent(0);
}

void PropertyRow::setNames(const char* name, const char* label, const char* typeName)
{
	name_ = name;
	YASLI_ASSERT(name);
	label_ = label ? label : "";
	YASLI_ASSERT(strlen(typeName));
	typeName_ = typeName;
}

PropertyRow* PropertyRow::childByIndex(int index)
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

const PropertyRow* PropertyRow::childByIndex(int index) const
{
	if(index >= 0 && index < int(children_.size()))
		return children_[index];
	else
		return 0;
}

void PropertyRow::_setExpanded(bool expanded)
{
    expanded_ = expanded;
	int numChildren = (int)children_.size();

	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* child = children_[i];
		if(child->pulledUp())
			child->_setExpanded(expanded);
	}

	layoutChanged_ = true;
	setLayoutChangedToChildren();
	
}

struct SetExpandedOp {
    bool expanded_;
    SetExpandedOp(bool expanded) : expanded_(expanded) {}
    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(row->canBeToggled(tree))
            row->_setExpanded(expanded_);
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::setExpandedRecursive(PropertyTree* tree, bool expanded)
{
	if(canBeToggled(tree))
		_setExpanded(expanded);

	SetExpandedOp op(expanded);
	scanChildren(op, tree);
}

int PropertyRow::childIndex(PropertyRow* row)
{
	YASLI_ASSERT(row);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ESCAPE(it != children_.end(), return -1);
	return (int)std::distance(children_.begin(), it);
}

bool PropertyRow::isChildOf(const PropertyRow* row) const
{
	const PropertyRow* p = parent();
	while(p){
		if(p == row)
			return true;
		p = p->parent();
	}
	return false;
}

void PropertyRow::add(PropertyRow* row)
{
	children_.push_back(row);
	row->setParent(this);
}

void PropertyRow::insertAfterUpdated(PropertyRow* row, int index)
{
	row->setParent(this);
	if(!index){
		children_.insert(children_.begin(), row);
		return;
	}
	for(Rows::iterator i = children_.begin(); i != children_.end(); ++i)
		if((*i)->updated_)
			if(!--index){
				children_.insert(i + 1, row);
				return;
			}
	children_.push_back(row);
}

void PropertyRow::addAfter(PropertyRow* row, PropertyRow* after)
{
	iterator it = std::find(children_.begin(), children_.end(), after);
	if(it != children_.end()){
		++it;
		children_.insert(it, row);
	}
	else{
		children_.push_back(row);
	}

	row->setParent(this);
}

void PropertyRow::assignRowState(const PropertyRow& row, bool recurse)
{
	expanded_ = row.expanded_;
	selected_ = row.selected_;
    if(recurse){
		int numChildren = (int)children_.size();
		for (int i = 0; i < numChildren; ++i) {
            PropertyRow* child = children_[i].get();
            YASLI_ESCAPE(child, continue);
			int unusedIndex;
            const PropertyRow* rhsChild = row.findFromIndex(&unusedIndex, child->name(), child->typeName(), i);
            if(rhsChild)
                child->assignRowState(*rhsChild, true);
        }
    }
}

void PropertyRow::assignRowProperties(PropertyRow* row)
{
    YASLI_ESCAPE(row, return);
	parent_ = row->parent_;
	
	userReadOnly_ = row->userReadOnly_;
	userReadOnlyRecurse_ = row->userReadOnlyRecurse_;
	userFixedWidget_ = row->userFixedWidget_;
	pulledUp_ = row->pulledUp_;
	pulledBefore_ = row->pulledBefore_;
	size_ = row->size_;
	pos_ = row->pos_;
	plusSize_ = row->plusSize_;
	textPos_ = row->textPos_;
	textSizeInitial_ = row->textSizeInitial_;
	textHash_ = row->textHash_;
	textSize_ = row->textSize_;
	widgetPos_ = row->widgetPos_;
	widgetSize_ = row->widgetSize_;
	userWidgetSize_ = row->userWidgetSize_;

    assignRowState(*row, false);
}

void PropertyRow::replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren)
{
	Rows::iterator it = std::find(children_.begin(), children_.end(), oldRow);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end()){
		newRow->assignRowProperties(*it);
		newRow->labelChanged_ = true;
		if(preserveChildren)
			(*it)->swapChildren(newRow);
		*it = newRow;
	}
}

void PropertyRow::erase(PropertyRow* row)
{
	row->setParent(0);
	Rows::iterator it = std::find(children_.begin(), children_.end(), row);
	YASLI_ASSERT(it != children_.end());
	if(it != children_.end())
		children_.erase(it);
}

void PropertyRow::swapChildren(PropertyRow* row)
{
	children_.swap(row->children_);
	iterator it;
	for( it = children_.begin(); it != children_.end(); ++it)
		(**it).setParent(this);
	for( it = row->children_.begin(); it != row->children_.end(); ++it)
		(**it).setParent(row);
}

void PropertyRow::eraseOld()
{
	for(Rows::iterator i = children_.begin(); i != children_.end();)
		if(!(*i)->updated_)
			i = children_.erase(i);
		else
			++i;
}

void PropertyRow::eraseOldRecursive()
{
	updated_ = false;
	for(Rows::iterator i = children_.begin(); i != children_.end();)
		if(!(*i)->updated_)
			i = children_.erase(i);
		else{
			(*i)->eraseOldRecursive();
			++i;
		}
}


std::size_t PropertyRow::countUpdated() const
{
	int counter = 0;
	for(Rows::const_iterator i = begin(); i != end(); ++i)
		if((*i)->updated_)
			++counter;
	return counter;
}

void PropertyRow::addBefore(PropertyRow* row, PropertyRow* before)
{
	if(before == 0)
		children_.push_back(row);
	else{
		iterator it = std::find(children_.begin(), children_.end(), before);
		if(it != children_.end())
			children_.insert(it, row);
		else
			children_.push_back(row);
	}
	row->setParent(this);
}

yasli::wstring PropertyRow::valueAsWString() const
{
    return toWideChar(valueAsString().c_str());
}

yasli::string PropertyRow::valueAsString() const
{
	return yasli::string();
}

SharedPtr<PropertyRow> PropertyRow::clone(ConstStringList* constStrings) const
{
	PropertyRow::setConstStrings(constStrings);
	yasli::BinOArchive oa;
	SharedPtr<PropertyRow> self(const_cast<PropertyRow*>(this));
	oa(self, "row", "Row");

	yasli::BinIArchive ia;
	ia.open(oa);
	SharedPtr<PropertyRow> clonedRow;
	ia(clonedRow, "row", "Row");
	PropertyRow::setConstStrings(0);
	return clonedRow;
}

void PropertyRow::YASLI_SERIALIZE_METHOD(Archive& ar)
{
	serializeValue(ar);

	ar(ConstStringWrapper(constStrings_, name_), "name", "name");
	ar(ConstStringWrapper(constStrings_, label_), "label", "label");
	ar(ConstStringWrapper(constStrings_, typeName_), "type", "type");
	ar(reinterpret_cast<std::vector<SharedPtr<PropertyRow> >&>(children_), "children", "!^children");	
	if(ar.isInput()){
		labelChanged_ = true;
		layoutChanged_ = true;
		PropertyRow::iterator it;
		for(it = begin(); it != end(); ){
			PropertyRow* row = *it;
			if(row){
				row->setParent(this);
				++it;
			}
			else{
				YASLI_ASSERT("Missing property row");
				it = erase(it);
			}
		}
	}
}

bool PropertyRow::onActivate(PropertyTree* tree, bool force)
{
    return tree->spawnWidget(this, force);
}

void PropertyRow::setLabelChanged() 
{ 
	for(PropertyRow* row = this; row != 0; row = row->parent())
		row->labelChanged_ = true;
}

void PropertyRow::setLayoutChanged()
{
	layoutChanged_ = true;
}

void PropertyRow::setLabelChangedToChildren()
{
	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		children_[i]->labelChanged_ = true;
		children_[i]->setLabelChangedToChildren();
	}
}

void PropertyRow::setLayoutChangedToChildren()
{
	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		children_[i]->layoutChanged_ = true;
		children_[i]->setLayoutChangedToChildren();
	}
}

void PropertyRow::setLabel(const char* label) 
{
	if (!label)
		label = "";
	if (label_ != label) {
		label_ = label;
		setLabelChanged(); 
	}
}

void PropertyRow::updateLabel(const PropertyTree* tree, int index)
{
	if (!labelChanged_) {
		if (pulledUp_)
		parent()->hasPulled_ = true;
		return;
	}

	hasPulled_ = false;

	int numChildren = (int)children_.size();
	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		row->updateLabel(tree, i);
	}

	parseControlCodes(label_, true);
	visible_ = *labelUndecorated_ != '\0' || userFullRow_ || pulledUp_ || isRoot();
	if (userHideChildren_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = children_[i];
			if (row->pulledUp())
				continue;
			row->visible_ = false;
		}
	}

	if(pulledContainer())
		pulledContainer()->_setExpanded(expanded());

	layoutChanged_ = true;
	labelChanged_ = false;
}

struct ResetSerializerOp{
    ScanResult operator()(PropertyRow* row)
    {
        row->setSerializer(Serializer());
        return SCAN_CHILDREN_SIBLINGS;
    }
};

void PropertyRow::parseControlCodes(const char* ptr, bool changeLabel)
{
	if (changeLabel) {
		userFullRow_ = false;
		pulledUp_ = false;
		pulledBefore_ = false;
		userReadOnly_ = false;
		userReadOnlyRecurse_ = false;
		userFixedWidget_ = false;
		userWidgetSize_ = -1;
		userHideChildren_ = false;
	}

	while(true){
		if(*ptr == '^'){
			if(parent() && !parent()->isRoot()){
				if(pulledUp_)
					pulledBefore_ = true;
				pulledUp_ = true;
				parent()->hasPulled_ = true;

				if(pulledUp() && isContainer())
					parent()->setPulledContainer(this);
			}
		}
		else if(*ptr == '+')
			_setExpanded(true);
		else if(*ptr == '<')
			userFullRow_ = true;
		else if(*ptr == '>'){
			userFixedWidget_ = true;
			const char* p = ++ptr;
			while(*p >= '0' && *p <= '9')
				++p;
			if(*p == '>'){
				userWidgetSize_ = atoi(ptr);
				ptr = ++p;
			}
			continue;
		}
		else if(*ptr == '-')
			userHideChildren_ = true;
		else if(*ptr == '~'){
            ResetSerializerOp op;
            scanChildren(op);
		}
		else if(*ptr == '!'){
			if(userReadOnly_)
				userReadOnlyRecurse_ = true;
			userReadOnly_ = true;
		}
		else if(*ptr == '['){
			++ptr;
			PropertyRow::iterator it;
			for(it = children_.begin(); it != children_.end(); ++it)
				(*it)->parseControlCodes(ptr, false);

			int counter = 1;
			while(*ptr){
				if(*ptr == ']' && !--counter)
					break;
				else if(*ptr == '[')
					++counter;
				++ptr;
			}
		}
		else
			break;
		++ptr;
	}

	if (changeLabel)
		labelUndecorated_ = ptr;

	labelChanged();
}

const char* PropertyRow::typeNameForFilter(PropertyTree* tree) const
{
	return typeName();
}

void PropertyRow::updateTextSizeInitial(const PropertyTree* tree, int index)
{
	char containerLabel[16] = "";
	const char* text = rowText(containerLabel, tree, index);
	if(text[0] == '\0') {
		textSizeInitial_ = 0;
		textHash_ = 0;
	}
	else{
		unsigned hash = calcHash(text);
		hash = calcHash(FONT_NORMAL, hash);
		if(hash != textHash_){
			textSizeInitial_ = tree->ui()->textWidth(text, FONT_NORMAL) + 3;
			textHash_ = hash;
		}
	}
}

void PropertyRow::calculateMinimalSize(const PropertyTree* tree, int posX, bool force, int* _extraSize, int index)
{
	PropertyRow* nonPulled = nonPulledParent();
	if (!layoutChanged_ && !force && !nonPulled->layoutChanged_) {
		DEBUG_TRACE_ROW("... skipping size for %s", label());
		return;
	}

	plusSize_ = 0;
	if(isRoot())
		expanded_ = true;
	else{
		if(nonPulled->isRoot() || (tree->compact() && nonPulled->parent()->isRoot()))
			_setExpanded(true);
		else if(!pulledUp())
			plusSize_ = tree->tabSize();

		if(parent()->pulledUp())
			pulledBefore_ = false;

		if(!visible(tree) && !(isContainer() && pulledUp())){
			size_ = Point(0, 0);
			DEBUG_TRACE_ROW("row '%s' got zero size", label());
			layoutChanged_ = false;
			return;
		}
	}

	widgetSize_ = widgetSizeMin(tree);

	updateTextSizeInitial(tree, index);

	size_ = Point(textSizeInitial_ + widgetSizeMin(tree), isRoot() ? 0 : tree->_defaultRowHeight() + floorHeight());

	pos_.setX(posX);
	posX += plusSize_;

	int freePulledChildren = 0;
	int extraSizeStorage = 0;
	int& extraSize = !pulledUp() || !_extraSize ? extraSizeStorage : *_extraSize;
	if(!pulledUp()){
		int minTextSize = 0;
		int minimalWidth = 0;
		calcPulledRows(&minTextSize, &freePulledChildren, &minimalWidth, tree, index);
		DEBUG_TRACE_ROW("%s minTextSize: %i, minimalWidth: %i", label(), minTextSize, minimalWidth);
		size_.setX(minimalWidth);
		extraSize = (tree->rightBorder() - tree->leftBorder()) - minimalWidth - posX;
		DEBUG_TRACE_ROW("%s extraSize 0: %i", label(), extraSize);

		float textScale = 1.0f;
		bool hideOwnText = false;
		if(extraSize < 0){
			// hide container item text first
			if (parent() && parent()->isContainer()){
				extraSize += textSizeInitial_;
				minTextSize -= textSizeInitial_;
				hideOwnText = true;
			}

			textScale = minTextSize ? clamp(1.0f - float(-extraSize) / minTextSize, 0.0f, 1.0f) : 0;
		}
		setTextSize(tree, index, textScale);

		if (hideOwnText) {
			textSize_ = 0;
			DEBUG_TRACE_ROW("%s hideOwnText", label());
		}
	}

	DEBUG_TRACE_ROW("%s extraSize 1: %i", label(), extraSize);

	WidgetPlacement widgetPlace = widgetPlacement();
	if(widgetPlace == WIDGET_AFTER_NAME && !tree->fullRowContainers())
		widgetPlace = WIDGET_VALUE;

	if(widgetPlace == WIDGET_ICON){
		widgetPos_ = widgetSize_ ? posX : -1000;
		posX += widgetSize_;
		textPos_ = posX;
		posX += textSize_;
	}

	int numChildren = (int)children_.size();
	if (hasPulled_) {
		for (int i = 0; i < numChildren; ++i) {
			PropertyRow* row = children_[i];
			if(row->visible(tree) && row->pulledBefore()){
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
				posX += row->size_.x();
			}
		}
	}

	if(widgetPlace != WIDGET_ICON){
		textPos_ = posX;
		posX += textSize_;
	}

	if(widgetPlace == WIDGET_AFTER_NAME){
		widgetPos_ = posX;
		posX += widgetSize_;
	}

	if(!pulledUp() && extraSize > 0){ // align widget value to value column
		if(!isFullRow(tree)){
			int oldX = posX;
			int newX = max(int(tree->leftBorder() + xround((tree->rightBorder() - tree->leftBorder())* (1.f - tree->valueColumnWidth()))), posX);
			int xDelta = newX - oldX;
			if (xDelta <= extraSize){
				extraSize -= xDelta;
				posX = newX;
			}
			else{
				posX += extraSize;
				extraSize = 0;
			}
		}
	}
	if (freePulledChildren > 0)
		extraSize = extraSize / freePulledChildren;

	if (widgetPlace == WIDGET_VALUE) {
		if(widgetSize_ && !isWidgetFixed() && extraSize > 0) {
			DEBUG_TRACE_ROW("%s widget extraSize: %i", label(), extraSize);
			widgetSize_ += extraSize;
		}

		widgetPos_ = posX;
		DEBUG_TRACE_ROW("textSize: %i widgetPos: %i", int(textSize_), int(widgetPos_));
		posX += widgetSize_;
		int delta = tree->rightBorder() - posX;
		if(delta > 0 && delta < 4)
			widgetSize_ += delta;
	}

	size_.setX(textSize_ + widgetSize_);

	for (int i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if(!row->visible(tree)) {
			DEBUG_TRACE_ROW("skipping invisible child: %s", row->label());
			continue;
		}
		if(row->pulledUp()){
			if(!row->pulledBefore()){
				if(!widgetPos_)
					widgetPos_ = posX;
				row->calculateMinimalSize(tree, posX, force, &extraSize, i);
				posX += row->size_.x();
			}
			size_.setX(size_.x() + row->size_.x());
			size_.setY(max(size_.y(), row->size_.y()));
		}
		else if(expanded())
			row->calculateMinimalSize(tree, nonPulled->plusRect(tree).right(), force, &extraSize, i);
	}

	if (widgetPlace == WIDGET_AFTER_PULLED)
		widgetPos_ = posX;

	if(!pulledUp())
		size_.setX(tree->rightBorder() - pos_.x());
	DEBUG_TRACE_ROW("calculateMinimalSize: '%s' %i %i (%s)", label(), size_.x(), size_.y(), isRoot() ? "root" : "non-root");
	layoutChanged_ = false;
}

void PropertyRow::adjustVerticalPosition(const PropertyTree* tree, int& totalHeight)
{
	pos_.setY(totalHeight);
	if(!pulledUp())
		totalHeight += size_.y();
	else{
		pos_.setY(parent()->pos_.y());
		expanded_ = parent()->expanded();
	}
	PropertyRow* nonPulled = nonPulledParent();

	DEBUG_TRACE_ROW("adjustRect: %s %i %i %i %i, totalHeight: %i %s", label(), pos_.x(), pos_.y(), size_.x(), size_.y(), totalHeight, pulledUp() ? "pulled" : "");

	if (expanded_ || hasPulled_) {
		for(PropertyRows::iterator it = children_.begin(); it != children_.end(); ++it){
			PropertyRow* row = *it;
			if(row->visible(tree) && (nonPulled->expanded() || row->pulledUp()))
				row->adjustVerticalPosition(tree, totalHeight);
		}
	}
}

void PropertyRow::setTextSize(const PropertyTree* tree, int index, float mult)
{
	updateTextSizeInitial(tree, index);

	textSize_ = xround(textSizeInitial_ * mult);

	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if(row->pulledUp())
			row->setTextSize(tree, 0, mult);
	}
}

void PropertyRow::calcPulledRows(int* minTextSize, int* freePulledChildren, int* minimalWidth, const PropertyTree *tree, int index) 
{
	updateTextSizeInitial(tree, index);

	*minTextSize += textSizeInitial_;
	if(widgetPlacement() == WIDGET_VALUE && !isWidgetFixed())
		*freePulledChildren += 1;
	*minimalWidth += textSizeInitial_ + widgetSizeMin(tree);

	size_t numChildren = children_.size();
	for (size_t i = 0; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if(row->pulledUp())
			row->calcPulledRows(minTextSize, freePulledChildren, minimalWidth, tree, index);
	}
}

PropertyRow* PropertyRow::findSelected()
{
    if(selected())
        return this;
    iterator it;
    for(it = children_.begin(); it != children_.end(); ++it){
        PropertyRow* result = (*it)->findSelected();
        if(result)
            return result;
    }
    return 0;
}

PropertyRow* PropertyRow::find(const char* name, const char* typeName)
{
	iterator it;
	for(it = children_.begin(); it != children_.end(); ++it){
		PropertyRow* row = *it;
		if((row->name() == name || strcmp(row->name(), name) == 0) &&
		   (typeName == 0 || (row->typeName() != 0 && strcmp(row->typeName(), typeName) == 0)))
			return row;
	}
	return 0;
}

PropertyRow* PropertyRow::findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex, bool checkUpdated) const
{
	int numChildren = (int)children_.size();

	for (int i = startIndex; i < numChildren; ++i) {
		PropertyRow* row = children_[i];
		if((row->name() == name || strcmp(row->name(), name) == 0) && (row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0) && (!checkUpdated || !row->updated_)){
			*outIndex = i;
			return row;
		}
	}

	startIndex = min(startIndex, numChildren);
	for (int i = 0; i < startIndex; ++i) {
		PropertyRow* row = children_[i];
		if((row->name() == name || strcmp(row->name(), name) == 0) && (row->typeName() == typeName || strcmp(row->typeName(), typeName) == 0) && (!checkUpdated || !row->updated_)){
			*outIndex = i;
			return row;
		}
	}

	*outIndex = -1;
	return 0;
}

bool PropertyRow::onKeyDown(PropertyTree* tree, const KeyEvent* ev)
{
	using namespace property_tree;
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler menuHandler(tree, container);
		menuHandler.element = this;
		if(ev->key() == KEY_DELETE && ev->modifiers() == 0) {
			menuHandler.onMenuChildRemove();
			return true;
		}
		else if(ev->key() == KEY_INSERT && ev->modifiers() == MODIFIER_SHIFT){
			menuHandler.onMenuChildInsertBefore();
			return true;
		}
	}
	return false;
}

struct ExpansionMenuHandler : PropertyRowMenuHandler
{
	PropertyTree* tree;
	ExpansionMenuHandler(PropertyTree* tree)
	: tree(tree)
	{
	}

	void onMenuExpand()
	{
		tree->expandAll();
	}
	void onMenuCollapse()
	{
		tree->collapseAll();
	}
};

bool PropertyRow::onContextMenu(property_tree::IMenu &menu, PropertyTree* tree)
{
	if(parent() && parent()->isContainer()){
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parent());
		ContainerMenuHandler* handler = new ContainerMenuHandler(tree, container);
		handler->element = this;
		tree->addMenuHandler(handler);
		if(!container->isFixedSize()){
		    if(!menu.isEmpty())
			    menu.addSeparator();

				menu.addAction("Insert Before", "Shift+Insert",
					container->userReadOnly() ? MENU_DISABLED : 0, handler, &ContainerMenuHandler::onMenuChildInsertBefore);
				
				menu.addAction("Remove", "Shift+Delete",
					container->userReadOnly() ? MENU_DISABLED : 0, handler, &ContainerMenuHandler::onMenuChildRemove);
		}
	}

	if(hasVisibleChildren(tree)){
		if(!menu.isEmpty())
			menu.addSeparator();

		ExpansionMenuHandler* handler = new ExpansionMenuHandler(tree);
		menu.addAction("Expand", 0, handler, &ExpansionMenuHandler::onMenuExpand);
		menu.addAction("Collapse", 0, handler, &ExpansionMenuHandler::onMenuCollapse);
		tree->addMenuHandler(handler);
	}

	return !menu.isEmpty();
}

int PropertyRow::level() const
{
    int result = 0;
    const PropertyRow* row = this;
    while(row){
        row = row->parent();
        ++result;
    }
    return result;
}

PropertyRow* PropertyRow::nonPulledParent()
{
	PropertyRow* row = this;
	while(row->pulledUp())
		row = row->parent();
	return row;
}

bool PropertyRow::pulledSelected() const
{
	if(selected())
		return true;
	const PropertyRow* row = this;
	while(row->pulledUp()){
		row = row->parent();
		if(row->selected())
			return true;
	}
	return false;
}


Font PropertyRow::rowFont(const PropertyTree* tree) const
{
	return hasVisibleChildren(tree) || isContainer() ? FONT_BOLD : FONT_NORMAL;
}

void PropertyRow::drawRow(IDrawContext& context, const PropertyTree* tree, int index, bool selectionPass)
{
	Rect rowRect = rect();
	Rect selectionRect;
	if(!pulledUp())
		selectionRect = rowRect.adjusted(plusSize_ - (tree->compact() ? 1 : 2), -2, 1, 1);
	else
		selectionRect = rowRect.adjusted(-1, 0, 1, -1);

	if (selectionPass) {
		if (selected()) {
			// drawing a selection rectangle
			context.drawSelection(selectionRect, false);
		}
		else{
			bool pulledChildrenSelected = false;

			for (size_t i = 0; i < children_.size(); ++i) {
				PropertyRow* child = children_[i];
				if (!child)
					continue;
				if ((child->pulledBefore() || child->pulledUp()) && child->selected())
					pulledChildrenSelected = true;
			}

			if (pulledChildrenSelected) {
				context.drawSelection(selectionRect, true);
				// draw rectangle around parent of selected pulled row
			}
		}
	}
	else{
		context.widgetRect = widgetRect(tree);
		context.lineRect = floorRect(tree);
		context.captured = tree->_isCapturedRow(this);
		context.pressed = tree->_pressedRow() == this;

		if(!tree->compact() || !parent()->isRoot()){
			if(hasVisibleChildren(tree)){
				context.drawPlus(plusRect(tree), expanded(), selected(), expanded());
			}
		}

		// custom drawing
		if(!isStatic() && context.widgetRect.isValid())
			redraw(context);

		if(textSize_ > 0){
			char containerLabel[16] = "";
			yasli::string text = rowText(containerLabel, tree, index);
			context.drawLabel(text.c_str(), FONT_NORMAL, textRect(tree), pulledSelected());
		}
	}
}

bool PropertyRow::visible(const PropertyTree* tree) const
{
	if (tree->_isDragged(this))
		return false;
	return ((visible_ || !tree->hideUntranslated()) && (matchFilter_ || belongsToFilteredRow_));
}

bool PropertyRow::canBeToggled(const PropertyTree* tree) const
{
	if(!visible(tree))
		return false;
	if((tree->compact() && (parent() && parent()->isRoot())) || (isContainer() && pulledUp()) || !hasVisibleChildren(tree))
		return false;
	return !empty();
}

bool PropertyRow::canBeDragged() const
{
	if(parent()){
		if(parent()->isContainer())
			return true;
	}
	return false;
}

bool PropertyRow::canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const
{
	YASLI_ASSERT(parentRow);

	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();

	if(parentRow->isContainer()){
		const PropertyRowContainer* container = static_cast<const PropertyRowContainer*>(parentRow);
				
		if((container->isFixedSize() || container->userReadOnly()) && parent() != parentRow)
			return false;

		if(beforeChild && beforeChild->parent() != parentRow)
			return false;

		const PropertyRow* defaultRow = container->defaultRow(tree->model());
		if(defaultRow && strcmp(defaultRow->typeName(), typeName()) == 0)
			return true;
	}
	return false;	
}

void PropertyRow::dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before)
{
	SharedPtr<PropertyRow> ref(this);

	PropertyTreeModel* model = tree->model();
	PropertyTreeModel::UpdateLock lock = model->lockUpdate();
	if(parentRow->pulledContainer())
		parentRow = parentRow->pulledContainer();
	if(parentRow->isContainer()){
		tree->model()->rowAboutToBeChanged(tree->model()->root()); // FIXME: select optimal row
		setSelected(false);
		PropertyRowContainer* container = static_cast<PropertyRowContainer*>(parentRow);
		PropertyRow* oldParent = parent();
		TreePath oldParentPath = tree->model()->pathFromRow(oldParent);
		oldParent->erase(this);
		if(before)
			parentRow->addBefore(this, cursorRow);
		else
			parentRow->addAfter(this, cursorRow);
		model->selectRow(this, true);
		TreePath thisPath = tree->model()->pathFromRow(this);
		TreePath parentRowPath = tree->model()->pathFromRow(parentRow);
		oldParent = tree->model()->rowFromPath(oldParentPath);
		if (oldParent)
			model->rowChanged(oldParent); // after this call we can get invalid this
		if(PropertyRow* newThis = tree->model()->rowFromPath(thisPath)) {
			TreeSelection selection;
			selection.push_back(thisPath);
			model->setSelection(selection);

			// we use path to obtain new row
			tree->ensureVisible(newThis);
			model->rowChanged(newThis); // after this call row pointers are invalidated
		}
		parentRow = tree->model()->rowFromPath(parentRowPath);
		if (parentRow)
			model->rowChanged(parentRow); // after this call row pointers are invalidated
	}
}

void PropertyRow::intersect(const PropertyRow* row)
{
	setMultiValue(multiValue() || row->multiValue() || valueAsString() != row->valueAsString());


	int indexSource = 0;
	for(int i = 0; i < int(children_.size()); ++i)
	{
		PropertyRow* testRow = children_[i];
		PropertyRow* matchingRow = row->findFromIndex(&indexSource, testRow->name_, testRow->typeName_, indexSource);
		++indexSource;
		if (matchingRow == 0) {
			children_.erase(children_.begin() + i);
			--i;
		}	
		else {
			children_[i]->intersect(matchingRow);
		}
	}
}

const char* PropertyRow::rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, int index) const
{
	if(parent() && parent()->isContainer()){
		if (tree->showContainerIndices()) {
            sprintf(containerLabelBuffer, " %i.", index);
			return containerLabelBuffer;
		}
		else
			return "";
	}
	else
		return labelUndecorated() ? labelUndecorated() : "";
}

bool PropertyRow::hasVisibleChildren(const PropertyTree* tree, bool internalCall) const
{
	if(empty() || (!internalCall && pulledUp()))
		return false;

	PropertyRow::const_iterator it;
	for(it = children_.begin(); it != children_.end(); ++it){
		const PropertyRow* child = *it;
		if(child->pulledUp()){
            if(child->hasVisibleChildren(tree, true))
                return true;
        }
        else if(child->visible(tree))
                return true;
	}
	return false;
}

const PropertyRow* PropertyRow::hit(const PropertyTree* tree, Point point) const
{
  return const_cast<PropertyRow*>(this)->hit(tree, point);
}

PropertyRow* PropertyRow::hit(const PropertyTree* tree, Point point)
{
    bool expanded = this->expanded();
    if(isContainer() && pulledUp())
        expanded = parent() ? parent()->expanded() : true;
    bool onlyPulled = !expanded;
    PropertyRow::const_iterator it;
    for(it = children_.begin(); it != children_.end(); ++it){
        PropertyRow* child = *it;
		if (!child->visible(tree))
			continue;
        if(!onlyPulled || child->pulledUp())
            if(PropertyRow* result = child->hit(tree, point))
                return result;
    }
	if (Rect(pos_.x(), pos_.y(), size_.x(), size_.y()).contains(point))
        return this;
    return 0;
}

PropertyRow* PropertyRow::findByAddress(const void* addr)
{
    if(serializer_.pointer() == addr)
        return this;
    else{
        Rows::iterator it;
        for(it = children_.begin(); it != children_.end(); ++it){
            PropertyRow* result = it->get()->findByAddress(addr);
            if(result)
                return result;
        }
    }
    return 0;
}

struct GetVerticalIndexOp{
    int index_;
    const PropertyRow* row_;

    GetVerticalIndexOp(const PropertyRow* row) : row_(row), index_(0) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(row == row_)
            return SCAN_FINISHED;
        if(row->visible(tree) && row->isSelectable() && !row->pulledUp())
            ++index_;
        return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
    }
};

int PropertyRow::verticalIndex(PropertyTree* tree, PropertyRow* row)
{
    GetVerticalIndexOp op(row);
	scanChildren(op, tree);
	return op.index_;
}


struct RowByVerticalIndexOp{
    int index_;
    PropertyRow* row_;

    RowByVerticalIndexOp(int index) : row_(0), index_(index) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(row->visible(tree) && !row->pulledUp() && row->isSelectable()){
            row_ = row;
            if(index_-- <= 0)
                return SCAN_FINISHED;
        }
        return row->expanded() ? SCAN_CHILDREN_SIBLINGS : SCAN_SIBLINGS;
    }
};

PropertyRow* PropertyRow::rowByVerticalIndex(PropertyTree* tree, int index)
{
    RowByVerticalIndexOp op(index);
	scanChildren(op, tree);
	return op.row_;
}

struct HorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    HorizontalIndexOp(PropertyRow* row) : row_(row), index_(0), pulledBefore_(row->pulledBefore()) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(!row->pulledUp())
            return SCAN_SIBLINGS;
        if(row->visible(tree) && row->isSelectable() && row->pulledUp() && row->pulledBefore() == pulledBefore_){
            index_ += pulledBefore_ ? -1 : 1;
            if(row == row_)
                return SCAN_FINISHED;
        }
        return SCAN_CHILDREN_SIBLINGS;
    }
};

int PropertyRow::horizontalIndex(PropertyTree* tree, PropertyRow* row)
{
	if(row == this)
		return 0;
    HorizontalIndexOp op(row);
	if(row->pulledBefore())
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.index_;
}

struct RowByHorizontalIndexOp{
    int index_;
    PropertyRow* row_;
    bool pulledBefore_;

    RowByHorizontalIndexOp(int index) : row_(0), index_(index), pulledBefore_(index < 0) {}

    ScanResult operator()(PropertyRow* row, PropertyTree* tree, int index)
    {
        if(!row->pulledUp())
            return SCAN_SIBLINGS;
        if(row->visible(tree) && row->isSelectable() && row->pulledUp() && row->pulledBefore() == pulledBefore_){
            row_ = row;
            if(pulledBefore_ ? ++index_ >= 0 : --index_ <= 0)
                return SCAN_FINISHED;
        }
        return SCAN_CHILDREN_SIBLINGS;
    }
};

PropertyRow* PropertyRow::rowByHorizontalIndex(PropertyTree* tree, int index)
{
	if(!index)
		return this;
    RowByHorizontalIndexOp op(index);
	if(index < 0)
		scanChildrenReverse(op, tree);
	else
		scanChildren(op, tree);
	return op.row_ ? op.row_ : this;
}

void PropertyRow::redraw(IDrawContext& context)
{

}

bool PropertyRow::isFullRow(const PropertyTree* tree) const
{
    if (tree->fullRowMode())
		return true;
	if (parent() && parent()->isContainer() && tree->fullRowContainers())
		return true;
	return userFullRow();
}

Rect PropertyRow::textRect(const PropertyTree* tree) const
{
	return Rect(textPos_, pos_.y(), textSize_ < textSizeInitial_ ? textSize_ - 1 : textSize_, tree->_defaultRowHeight());
}

Rect PropertyRow::widgetRect(const PropertyTree* tree) const
{
	return Rect(widgetPos_, pos_.y(), widgetSize_, tree->_defaultRowHeight());
}

Rect PropertyRow::plusRect(const PropertyTree* tree) const
{
	return Rect(pos_.x(), pos_.y(), plusSize_, tree->_defaultRowHeight());
}

Rect PropertyRow::floorRect(const PropertyTree* tree) const
{
	return Rect(textPos_, pos_.y() + tree->_defaultRowHeight(), size_.x() - (textPos_ - pos_.x()) , size_.y() - tree->_defaultRowHeight());
}

YASLI_CLASS(PropertyRow, PropertyRow, "Structure");

// ---------------------------------------------------------------------------


FORCE_SEGMENT(PropertyRowNumber)
FORCE_SEGMENT(PropertyRowStringList)
FORCE_SEGMENT(PropertyRowButton)
FORCE_SEGMENT(PropertyRowHorizontalLine)
/*
FORCE_SEGMENT(PropertyRowIconXPM)
FORCE_SEGMENT(PropertyRowFileOpen)
FORCE_SEGMENT(PropertyRowFileSave)
FORCE_SEGMENT(PropertyRowColor)
FORCE_SEGMENT(PropertyRowBitVector)
FORCE_SEGMENT(PropertyRowDecorators)
FORCE_SEGMENT(PropertyRowFileSelector)
FORCE_SEGMENT(PropertyRowHotkey)
FORCE_SEGMENT(PropertyRowSlider)
*/
