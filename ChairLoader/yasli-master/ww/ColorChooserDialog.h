/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Dialog.h"
#include "ww/Color.h"

namespace ww{

class ColorChooser;

class ColorChooserDialog : public ww::Dialog{
public:
	ColorChooserDialog(ww::Widget* parent, const Color& color = Color(0, 204, 0, 255), bool showColor = true, bool showAlpha = true);
	ColorChooserDialog(HWND parent, const Color& color = Color(0, 204, 0, 255), bool showColor = true, bool showAlpha = true);
	~ColorChooserDialog();

	void set(const Color& color);
	Color get() const{ return color_; }
protected:
	void initialize(const Color& color, bool showColor, bool showAlpha);
	void onChooserChanged();

	ColorChooser* chooser_;
	Color color_;
};

}

