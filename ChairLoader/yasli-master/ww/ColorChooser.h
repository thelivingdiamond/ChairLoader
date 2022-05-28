/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/VBox.h"
#include "ww/Color.h"

namespace ww{

class ColorRamp;
class ColorRect;
class Entry;
class Slider;

class ColorChooser : public VBox{
public:
	ColorChooser(int border = 0);

	void set(const Color& color);
	const Color& get() const{ return color_; }

	void setShowColor(bool showColor);
	bool showColor()const{ return showColor_; }
	void setShowAlpha(bool showAlpha);
	bool showAlpha()const{ return showAlpha_; }

	signal0& signalChanged() { return signalChanged_; }
protected:
	signal0 signalChanged_;
	void relayout();
	void onRampChanged();
	void onSliderChanged();
	void onEntryChanged();
	void onHexChanged();
	void updateEntries(const Color& color);
	void updateSliders(const Color& color);
	void updateHex(const Color& color);
	
	bool showColor_;
	bool showAlpha_;

	Entry* entryRed_;
	Entry* entryGreen_;
	Entry* entryBlue_;
	Entry* entryAlpha_;
	Entry* entryHex_;

	Slider* sliderRed_;
	Slider* sliderGreen_;
	Slider* sliderBlue_;
	Slider* sliderAlpha_;

	ColorRamp* ramp_;
	ColorRect* colorRect_;
	Color color_;
};

}

