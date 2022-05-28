/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Container.h"

namespace Win32{
    class Window;
};

namespace ww{

class SplitterImpl;

class Splitter: public Container{
public:
    Splitter(int splitterSize, int border, SplitterImpl* impl);
	~Splitter();
	static const int SPLITTER_WIDTH = 6;

    /// удалить все дочерние котролы
    void clear();
    /// изменить количество секций (мест для контролов)
    void resize(int newSize);
    /// добавить перед элементом beforeIndex (-1 = добавить в конец) 
    void add(Widget* widget, float position = 0.5f, bool fold = true, int beforeIndex = -1);
    /// удалить контрол по индексу
    void remove(int index, bool inFavourOfPrevious);
	/// заменить контрол
	void replace(Widget* oldWidget, Widget* newWidget);
	/// установить положение сплиттера
	void setSplitterPosition(float position, int splitterIndex = 0);
    void setFlat(bool flat){ flat_ = flat; }

	int splitterSpacing() const{ return splitterSpacing_; }
	float widgetPosition(Widget* widget) const;
	Widget* widgetByPosition(float position);
	Widget* widgetByScreenPoint(Vect2 point);
	Widget* widgetByIndex(int index);
	virtual bool vertical() = 0;

    // virtuals:
	void visitChildren(WidgetVisitor& visitor) const;
	void serialize(Archive& ar);
    // ^^

	void _updateVisibility();
	void _setParent(Container* container);
	void _setPosition(const Rect& position);
	void _arrangeChildren();
	void _relayoutParents();

	int splittersCount();
	Rect getSplitterRect(int splitterIndex);
protected:
	friend SplitterImpl;

	Win32::Window32* _window() const{ return window_; }

	void setPanePosition(int index, int poisitionInPixels);
	virtual int boxLength() const = 0;
	virtual Vect2 elementSize(Widget* widget) const = 0;
	virtual void setSplitterMinimalSize(const Vect2& size) = 0;
	virtual Rect rectByPosition(int start, int end) = 0;
	virtual int positionFromPoint(const Vect2 point) const = 0;

	int splitterWidth() const;


	struct Element{
		Element()
		: position(0.0f)
		, snappedPosition(0.0f)
		, fold(false)
		{
		}
		void serialize(Archive& ar);
		SharedPtr<ww::Widget> widget;
		Rect splitterRect;
		float position;
		float snappedPosition;
		bool fold;
	};

	typedef std::list<Element> Elements;
	Elements elements_;
    Win32::Window32* window_;
    bool flat_;
	int splitterSpacing_;
};

}

