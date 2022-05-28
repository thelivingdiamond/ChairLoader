/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"
#include "ww/_WidgetWithWindow.h"
#include "ww/_Enums.h"

namespace ww{

class LabelImpl;
class Label : public _WidgetWithWindow{
public:
	Label(const char* text = "Label", bool emphasis = false, int border = 0);

	/// виджет, которму передается фокус при нажатии хоткея из лейбла или клику по нему
	//void setMnemonicWidget(Widget* mnemonicWidget);

	void setAlignment(TextAlignHorizontal alignh, TextAlignVertical alignv = ALIGN_MIDDLE);

	void setExpandByContent(bool expand);
	bool expandByContent() const{ return expandByContent_; }

	void setEmphasis(bool emphasis);
	bool emphasis() const{ return emphasis_; }

	void setText(const char* text);
	const char* text() const{ return text_.c_str(); }

	void serialize(Archive& ar);
protected:
	LabelImpl* impl() const{ return reinterpret_cast<LabelImpl*>(_window()); }
	friend class LabelImpl;

	string text_;
	Widget* mnemonicWidget_;
	TextAlignHorizontal alignHorizontal_;
	TextAlignVertical alignVertical_;
	bool emphasis_;
	bool expandByContent_;
};

};


