/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#if !defined(__clang__) && defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
// GCC got support for override keyword in 4.8
# define override
#endif

#include <typeinfo>
#include <algorithm>
#include "yasli/Serializer.h"
#include "yasli/StringList.h"
#include "yasli/Pointers.h"
#include "yasli/Config.h"
#include "Factory.h"
#include "ConstStringList.h"
#include "IDrawContext.h"
#include "Rect.h"
#include "sigslot.h"

class QPainter;
namespace property_tree { 
struct KeyEvent; 
class IMenu;
class InplaceWidget;
}
using property_tree::KeyEvent;

using std::vector;
class PropertyTree;
class PropertyRow;
class PropertyTreeModel;

enum ScanResult {
	SCAN_FINISHED, 
	SCAN_CHILDREN, 
	SCAN_SIBLINGS, 
	SCAN_CHILDREN_SIBLINGS,
};

struct PropertyRowMenuHandler : sigslot::has_slots
{
public:

	virtual ~PropertyRowMenuHandler() {}

};

struct PropertyDragEvent
{
	::PropertyTree* tree;
	Point pos;
	Point start;
	Point lastDelta;
};

enum DragCheckBegin {
	DRAG_CHECK_IGNORE,
	DRAG_CHECK_SET,
	DRAG_CHECK_UNSET
};

class PropertyTreeTransaction;

class PropertyRow : public yasli::RefCounter
{
public:
	enum WidgetPlacement {
		WIDGET_NONE,
		WIDGET_ICON,
		WIDGET_AFTER_NAME,
		WIDGET_VALUE,
		WIDGET_AFTER_PULLED
	};

	typedef std::vector< yasli::SharedPtr<PropertyRow> > Rows;
	typedef Rows::iterator iterator;
	typedef Rows::const_iterator const_iterator;

	PropertyRow();
	virtual ~PropertyRow();

	void setNames(const char* name, const char* label, const char* typeName);

	bool selected() const{ return selected_; }
	void setSelected(bool selected) { selected_ = selected; }
	bool expanded() const{ return expanded_; }
	void _setExpanded(bool expanded); // use PropertyTree::expandRow
	void setExpandedRecursive(PropertyTree* tree, bool expanded);

	void setMatchFilter(bool matchFilter) { matchFilter_ = matchFilter; }
	bool matchFilter() const { return matchFilter_; }

	void setBelongsToFilteredRow(bool belongs) { belongsToFilteredRow_ = belongs; }
	bool belongsToFilteredRow() const { return belongsToFilteredRow_; }

	bool visible(const PropertyTree* tree) const;
	bool hasVisibleChildren(const PropertyTree* tree, bool internalCall = false) const;

	const PropertyRow* hit(const PropertyTree* tree, Point point) const;
	PropertyRow* hit(const PropertyTree* tree, Point point);
	PropertyRow* parent() { return parent_; }
	const PropertyRow* parent() const{ return parent_; }
	void setParent(PropertyRow* row) { parent_ = row; }
	bool isRoot() const { return !parent_; }
	int level() const;

	void add(PropertyRow* row);
	void insertAfterUpdated(PropertyRow* row, int index);
	void addAfter(PropertyRow* row, PropertyRow* after);
	void addBefore(PropertyRow* row, PropertyRow* before);

	template<class Op> bool scanChildren(Op& op);
	template<class Op> bool scanChildren(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenReverse(Op& op, PropertyTree* tree);
	template<class Op> bool scanChildrenBottomUp(Op& op, PropertyTree* tree);

	PropertyRow* childByIndex(int index);
	const PropertyRow* childByIndex(int index) const;
	int childIndex(PropertyRow* row);
	bool isChildOf(const PropertyRow* row) const;

	bool empty() const{ return children_.empty(); }
	iterator find(PropertyRow* row) { return std::find(children_.begin(), children_.end(), row); }
	PropertyRow* findFromIndex(int* outIndex, const char* name, const char* typeName, int startIndex, bool checkUpdated = false) const;
	PropertyRow* findByAddress(const void* addr);
	iterator begin() { return children_.begin(); }
	iterator end() { return children_.end(); }
	const_iterator begin() const{ return children_.begin(); }
	const_iterator end() const{ return children_.end(); }
	std::size_t count() const{ return children_.size(); }
	iterator erase(iterator it){ return children_.erase(it); }
	void clear(){ children_.clear(); }
	void erase(PropertyRow* row);
	void swapChildren(PropertyRow* row);
	void eraseOld();
	void eraseOldRecursive();
	std::size_t countUpdated() const;

	void assignRowState(const PropertyRow& row, bool recurse);
	void assignRowProperties(PropertyRow* row);
	void replaceAndPreserveState(PropertyRow* oldRow, PropertyRow* newRow, bool preserveChildren = true);

	const char* name() const{ return name_; }
	void setName(const char* name) { name_ = name; }
	const char* label() const { return label_; }
	const char* labelUndecorated() const { return labelUndecorated_; }
	void setLabel(const char* label);
	void setLabelChanged();
	void setLayoutChanged();
	void setLabelChangedToChildren();
	void setLayoutChangedToChildren();
	void updateLabel(const PropertyTree* tree, int index);
	void updateTextSizeInitial(const PropertyTree* tree, int index);
	virtual void labelChanged() {}
	void parseControlCodes(const char* label, bool changeLabel);
	const char* typeName() const{ return typeName_; }
	virtual const char* typeNameForFilter(PropertyTree* tree) const;
	void setTypeName(const char* typeName) { YASLI_ASSERT(strlen(typeName)); typeName_ = typeName; }
	const char* rowText(char (&containerLabelBuffer)[16], const PropertyTree* tree, int rowIndex) const;

	PropertyRow* findSelected();
	PropertyRow* find(const char* name, const char* typeName);
	void intersect(const PropertyRow* row);

	int verticalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByVerticalIndex(PropertyTree* tree, int index);
	int horizontalIndex(PropertyTree* tree, PropertyRow* row);
	PropertyRow* rowByHorizontalIndex(PropertyTree* tree, int index);

	virtual bool assignToPrimitive(void* object, size_t size) const{ return false; }
	virtual bool assignTo(const yasli::Serializer& ser) const{ return false; }
	virtual void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) { serializer_ = ser; }
	virtual yasli::string valueAsString() const;
	virtual yasli::wstring valueAsWString() const;

	int height() const{ return size_.y(); }

	virtual int widgetSizeMin(const PropertyTree*) const { return userWidgetSize() >= 0 ? userWidgetSize() : 0; } 
	virtual int floorHeight() const{ return 0; }

	void calcPulledRows(int* minTextSize, int* freePulledChildren, int* minimalWidth, const PropertyTree* tree, int index);
	void calculateMinimalSize(const PropertyTree* tree, int posX, bool force, int* _extraSize, int index);
	void setTextSize(const PropertyTree* tree, int rowIndex, float multiplier);
	void calculateTotalSizes(int* minTextSize);
	void adjustVerticalPosition(const PropertyTree* tree, int& totalHeight);

	virtual bool isWidgetFixed() const{ return userFixedWidget_ || widgetPlacement() != WIDGET_VALUE; }

	virtual WidgetPlacement widgetPlacement() const{ return WIDGET_NONE; }

	Rect rect() const{ return Rect(pos_.x(), pos_.y(), size_.x(), size_.y()); }
	Rect textRect(const PropertyTree* tree) const;
	Rect widgetRect(const PropertyTree* tree) const;
	Rect plusRect(const PropertyTree* tree) const;
	Rect floorRect(const PropertyTree* tree) const;
	void adjustHoveredRect(Rect& hoveredRect);
	Font rowFont(const PropertyTree* tree) const;

	void drawRow(IDrawContext& x, const PropertyTree* tree, int rowIndex, bool selectionPass);

	virtual void redraw(IDrawContext& context);
	virtual property_tree::InplaceWidget* createWidget(PropertyTree* tree) { return 0; }

	virtual bool isContainer() const{ return false; }
	virtual bool isPointer() const{ return false; }
	virtual bool isObject() const{ return false; }

	virtual bool isLeaf() const{ return false; }
	virtual void closeNonLeaf(const yasli::Serializer& ser) {}
	virtual bool isStatic() const{ return pulledContainer_ == 0; }
	virtual bool isSelectable() const{ return (!userReadOnly() && !userReadOnlyRecurse()) || (!pulledUp() && !pulledBefore()); }
	virtual bool activateOnAdd() const{ return false; }

	bool canBeToggled(const PropertyTree* tree) const;
	bool canBeDragged() const;
	bool canBeDroppedOn(const PropertyRow* parentRow, const PropertyRow* beforeChild, const PropertyTree* tree) const;
	void dropInto(PropertyRow* parentRow, PropertyRow* cursorRow, PropertyTree* tree, bool before);

	virtual bool onActivate(PropertyTree* tree, bool force);
	virtual bool onActivateRelease(PropertyTree* tree) { return false; }
	virtual bool onKeyDown(PropertyTree* tree, const KeyEvent* ev);
	virtual bool onMouseDown(PropertyTree* tree, Point point, bool& changed) { return false; }
    virtual void onMouseDrag(const PropertyDragEvent& e) {}
	virtual void onMouseStill(const PropertyDragEvent& e) {}
	virtual void onMouseUp(PropertyTree* tree, Point point) {}
	// "drag check" allows you to "paint" with the mouse through checkboxes to set all values at once
	virtual DragCheckBegin onMouseDragCheckBegin() { return DRAG_CHECK_IGNORE; }
	virtual bool onMouseDragCheck(PropertyTree* tree, bool value) { return false; }
	virtual bool onContextMenu(IMenu &menu, PropertyTree* tree);

	bool isFullRow(const PropertyTree* tree) const;

	// User states.
	// Assigned using control codes (characters in the beginning of label)
	// fixed widget doesn't expand automatically to occupy all available place
	bool userFixedWidget() const{ return userFixedWidget_; }
	bool userFullRow() const { return userFullRow_; }
	bool userReadOnly() const { return userReadOnly_; }
	bool userReadOnlyRecurse() const { return userReadOnlyRecurse_; }
	int userWidgetSize() const{ return userWidgetSize_; }

	// multiValue is used to edit properties of multiple objects simulateneously
	bool multiValue() const { return multiValue_; }
	void setMultiValue(bool multiValue) { multiValue_ = multiValue; }

	// pulledRow - is the one that is pulled up to the parents row
	// (created with ^ in the beginning of label)
	bool pulledUp() const { return pulledUp_; }
	bool pulledBefore() const { return pulledBefore_; }
	bool hasPulled() const { return hasPulled_; }
	bool pulledSelected() const;
	PropertyRow* nonPulledParent();
	void setPulledContainer(PropertyRow* container){ pulledContainer_ = container; }
	PropertyRow* pulledContainer() { return pulledContainer_; }
	const PropertyRow* pulledContainer() const{ return pulledContainer_; }

	yasli::SharedPtr<PropertyRow> clone(ConstStringList* constStrings) const;

	yasli::Serializer serializer() const{ return serializer_; }
    void setSerializer(const yasli::Serializer& ser) { serializer_ = ser; }
	virtual void serializeValue(yasli::Archive& ar) {}
	void YASLI_SERIALIZE_METHOD(yasli::Archive& ar);

	static void setConstStrings(ConstStringList* constStrings){ constStrings_ = constStrings; }

protected:
	void init(const char* name, const char* nameAlt, const char* typeName);

	const char* name_;
	const char* label_;
	const char* labelUndecorated_;
	const char* typeName_;
	yasli::Serializer serializer_;
	PropertyRow* parent_;
	Rows children_;

	unsigned int textHash_;

	// do we really need Point here? 
	Point pos_;
	Point size_;
	short int textPos_;
	short int textSizeInitial_;
	short int textSize_;
	short int widgetPos_; // widget == icon!
	short int widgetSize_;
	short int userWidgetSize_;
	unsigned char plusSize_;
	bool visible_ : 1;
	bool matchFilter_ : 1;
	bool belongsToFilteredRow_ : 1;
	bool expanded_ : 1;
	bool selected_ : 1;
	bool labelChanged_ : 1;
	bool layoutChanged_ : 1;
	bool userReadOnly_ : 1;
	bool userReadOnlyRecurse_ : 1;
	bool userFixedWidget_ : 1;
	bool userFullRow_ : 1;
	bool userHideChildren_ : 1;
	bool pulledUp_ : 1;
	bool pulledBefore_ : 1;
	bool hasPulled_ : 1;
	bool multiValue_ : 1;
	bool updated_ : 1;

	yasli::SharedPtr<PropertyRow> pulledContainer_;
	static ConstStringList* constStrings_;
	friend class PropertyOArchive;
	friend class PropertyIArchive;
};

typedef vector<yasli::SharedPtr<PropertyRow> > PropertyRows;

template<bool value>
struct StaticBool{
	enum { Value = value };
};

struct LessStrCmpPR
{
	bool operator()(const char* a, const char* b) const {
		return strcmp(a, b) < 0;
	}
};

typedef Factory<const char*, PropertyRow, Constructor0<PropertyRow>, LessStrCmpPR> PropertyRowFactory;

template<class Op>
bool PropertyRow::scanChildren(Op& op)
{
	Rows::iterator it;

	for(it = children_.begin(); it != children_.end(); ++it){
		ScanResult result = op(*it);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!(*it)->scanChildren(op))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildren(Op& op, PropertyTree* tree)
{
	int numChildren = (int)children_.size();
	for(int index = 0; index < numChildren; ++index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildren(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenReverse(Op& op, PropertyTree* tree)
{
	int numChildren = (int)children_.size();
	for(int index = numChildren - 1; index >= 0; --index){
		PropertyRow* child = children_[index];
		ScanResult result = op(child, tree, index);
		if(result == SCAN_FINISHED)
			return false;
		if(result == SCAN_CHILDREN || result == SCAN_CHILDREN_SIBLINGS){
			if(!child->scanChildrenReverse(op, tree))
				return false;
			if(result == SCAN_CHILDREN)
				return false;
		}
	}
	return true;
}

template<class Op>
bool PropertyRow::scanChildrenBottomUp(Op& op, PropertyTree* tree)
{
	size_t numChildren = children_.size();
	for(size_t i = 0; i < numChildren; ++i)
	{
		PropertyRow* child = children_[i];
		if(!child->scanChildrenBottomUp(op, tree))
			return false;
		ScanResult result = op(child, tree);
		if(result == SCAN_FINISHED)
			return false;
	}
	return true;
}


#define REGISTER_PROPERTY_ROW(DataType, RowType) \
	REGISTER_IN_FACTORY(PropertyRowFactory, yasli::TypeID::get<DataType>().name(), RowType); \
	YASLI_CLASS(PropertyRow, RowType, #DataType);


