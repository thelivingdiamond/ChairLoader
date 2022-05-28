/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include "yasli/Serializer.h"
#include "yasli/Object.h"
#include "yasli/Pointers.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/PropertyRow.h"
#include <QWidget>
#include <QPainter>
#include <vector>

class QLineEdit;
class QScrollBar;
class QPropertyTree;

namespace property_tree { 
class QDrawContext; 
class IMenu;
}

class DragWindow : public QWidget
{
	Q_OBJECT
public:
	DragWindow(QPropertyTree* tree);

	void set(QPropertyTree* tree, PropertyRow* row, const QRect& rowRect);
	void setWindowPos(bool visible);
	void show();
	void move(int deltaX, int deltaY);
	void hide();

	void drawRow(QPainter& p);
	void paintEvent(QPaintEvent* ev);

protected:
	bool useLayeredWindows_;
	PropertyRow* row_;
	QRect rect_;
	QPropertyTree* tree_;
	QPoint offset_;
};

class QPropertyTree : public QWidget, public PropertyTree
{
	Q_OBJECT
public:
	explicit QPropertyTree(QWidget* parent = 0);
	~QPropertyTree();

	const QFont& boldFont() const; 
	
	void setSizeHint(const QSize& size) { sizeHint_ = size; }

	void attachPropertyTree(PropertyTree* propertyTree) override;

public slots:
	void onAttachedTreeChanged();
public:
	// internal methods:
	QPoint _toScreen(Point point) const;
	void _drawRowValue(QPainter& p, const char* text, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;
	int _updateHeightsTime() const{ return updateHeightsTime_; }
	int _paintTime() const{ return paintTime_; }
	bool hasFocusOrInplaceHasFocus() const override;
	bool _isDragged(const PropertyRow* row) const override;

signals:
	void signalAboutToSerialize(yasli::Archive& ar);
	void signalChanged();
	void signalContinuousChange();
	void signalObjectChanged(const yasli::Object& obj);
	void signalSelected();
	void signalReverted();
	void signalPushUndo();
public slots:
    void onFilterChanged(const QString& str);
protected slots:
	void onScroll(int pos);
	void onMouseStillTimer();

protected:
	void onAboutToSerialize(yasli::Archive& ar) override { signalAboutToSerialize(ar); }
	void onChanged() override { signalChanged(); }
	void onContinuousChange() override { signalContinuousChange(); }
	void onSelected() override { signalSelected(); }
	void onReverted() override { signalReverted(); }
	void onPushUndo() override { signalPushUndo(); }

	void copyRow(PropertyRow* row) override;
	void pasteRow(PropertyRow* row) override;
	bool canBePasted(PropertyRow* destination) override;
	bool canBePasted(const char* destinationType) override;

	void interruptDrag() override;
	void defocusInplaceEditor() override;
	class DragController;

	void updateHeights() override;
	void repaint() override { update(); }
	void resetFilter() override { onFilterChanged(QString()); }

	QSize sizeHint() const override;
	void paintEvent(QPaintEvent* ev) override;
	void moveEvent(QMoveEvent* ev) override;
	void resizeEvent(QResizeEvent* ev) override;
	void mousePressEvent(QMouseEvent* ev) override;
	void mouseReleaseEvent(QMouseEvent* ev) override;
	void mouseDoubleClickEvent(QMouseEvent* ev) override;
	void mouseMoveEvent(QMouseEvent* ev) override;
	void wheelEvent(QWheelEvent* ev) override;
	void keyPressEvent(QKeyEvent* ev) override;
	void focusInEvent(QFocusEvent* ev) override;

	bool updateScrollBar() override;

	void setFilterMode(bool inFilterMode);
	void startFilter(const char* filter) override;

	void _arrangeChildren() override;

	void drawFilteredString(QPainter& p, const char* text, RowFilter::Type type, const QFont* font, const QRect& rect, const QColor& color, bool pathEllipsis, bool center) const;

	QScopedPointer<QLineEdit> filterEntry_; 
	
	QScrollBar* scrollBar_;
	QFont boldFont_;
	QSize sizeHint_;
	DragController* dragController_;
	QTimer* mouseStillTimer_;

	int updateHeightsTime_;
	int paintTime_;

	friend class property_tree::QDrawContext;
	friend class QUIFacade;
	friend struct PropertyTreeMenuHandler;
	friend class FilterEntry;
	friend class DragWindow;
};

yasli::wstring generateDigest(yasli::Serializer& ser);
