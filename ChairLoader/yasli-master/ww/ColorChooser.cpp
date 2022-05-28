/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Assert.h"
#include "ww/ColorRamp.h"
#include "ww/ColorChooser.h"
#include "ww/HBox.h"
#include "ww/VBox.h"
#include "ww/Slider.h"
#include "ww/ColorRect.h"
#include "ww/Label.h"
#include "ww/Entry.h"

namespace ww{

ColorChooser::ColorChooser(int border)
: VBox(5, border)
, showColor_(true)
, showAlpha_(true)
{
	relayout();
}

void ColorChooser::relayout()
{
	clear();

	sliderRed_ = 0;
	sliderGreen_ = 0;
	sliderBlue_ = 0;
	sliderAlpha_ = 0;
	entryRed_ = 0;
	entryGreen_ = 0;
	entryBlue_ = 0;
	entryAlpha_ = 0;
	entryHex_ = 0;
	ramp_ = 0;

	if(showColor_){
		ramp_ = new ColorRamp();
		ramp_->signalChanged().connect(this, &ColorChooser::onRampChanged);
		add(ramp_, PACK_FILL);
	}
	
	HBox* hbox = new HBox(5);
	add(hbox, PACK_COMPACT);
	{
        colorRect_ = new ColorRect();
		colorRect_->setRequestSize(64, 64);
		colorRect_->set(color_);
		hbox->add(colorRect_);

		VBox* vbox = new VBox();
		hbox->add(vbox, PACK_COMPACT);
		{
			if(showColor_){
				vbox->add(new Label("R"), PACK_FILL);
				vbox->add(new Label("G"), PACK_FILL);
				vbox->add(new Label("B"), PACK_FILL);
			}
			if(showAlpha_){
				vbox->add(new Label("A"), PACK_FILL);
			}
		}
		vbox = new VBox();
		hbox->add(vbox, PACK_FILL);
		{
			if(showColor_){
				sliderRed_ = new Slider();
				sliderRed_->setStepsCount(256);
				sliderRed_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderRed_, PACK_FILL);
				sliderGreen_ = new Slider();
				sliderGreen_->setStepsCount(256);
				sliderGreen_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderGreen_, PACK_FILL);
				sliderBlue_ = new Slider();
				sliderBlue_->setStepsCount(256);
				sliderBlue_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderBlue_, PACK_FILL);
			}
			if(showAlpha_){
				sliderAlpha_ = new Slider();
				sliderAlpha_->setStepsCount(256);
				sliderAlpha_->signalChanged().connect(this, &ColorChooser::onSliderChanged);
				vbox->add(sliderAlpha_, PACK_FILL);
			}
		}
		vbox = new VBox();
		hbox->add(vbox, PACK_COMPACT);
		vbox->setRequestSize(50, 10);
		{
			if(showColor_){
				entryRed_ = new Entry();
				entryRed_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryRed_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryRed_, PACK_FILL);

				entryGreen_ = new Entry();
				entryGreen_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryGreen_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryGreen_, PACK_FILL);

				entryBlue_ = new Entry();
				entryBlue_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryBlue_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryBlue_, PACK_FILL);
			}
			if(showAlpha_){
				entryAlpha_ = new Entry();
				entryAlpha_->signalChanged().connect(this, &ColorChooser::onEntryChanged);
				entryAlpha_->setTextAlign(ALIGN_CENTER);
				vbox->add(entryAlpha_, PACK_FILL);
			}
		}
	}
	if(showColor_ || showAlpha_){
		entryHex_ = new Entry();
		entryHex_->signalChanged().connect(this, &ColorChooser::onHexChanged);
		entryHex_->setTextAlign(ALIGN_CENTER);
		add(entryHex_);
	}
}

void ColorChooser::setShowColor(bool showColor)
{
	showColor_ = showColor;
	relayout();
}

void ColorChooser::setShowAlpha(bool showAlpha)
{
	showAlpha_ = showAlpha;
	relayout();
}

void ColorChooser::set(const Color& color)
{
	color_ = color;
	if(ramp_)
		ramp_->set(color);
	colorRect_->set(color);
	updateEntries(color);
	updateSliders(color);
	updateHex(color);
}

static void setEntryValue(Entry* entry, unsigned char value)
{
	char buf[16];
	sprintf_s(buf, sizeof(buf), "%i", value) ;
	entry->setText(buf);
}

void ColorChooser::updateEntries(const Color& color)
{
	if(showColor_){
		setEntryValue(entryRed_, color.r);
		setEntryValue(entryGreen_, color.g);
		setEntryValue(entryBlue_, color.b);
	}
	if(showAlpha_){
		setEntryValue(entryAlpha_, color.a);
	}
}

void ColorChooser::updateHex(const Color& color)
{
	if(showColor_ || showAlpha_){
		char buf[16];
		sprintf_s(buf, sizeof(buf), "%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
		entryHex_->setText(buf);
	}
}

void ColorChooser::updateSliders(const Color& color)
{
	if(showColor_){
		sliderRed_->setValue(color.r / 255.0f);
		sliderGreen_->setValue(color.g / 255.0f);
		sliderBlue_->setValue(color.b / 255.0f);
	}
	if(showAlpha_){
		sliderAlpha_->setValue(color.a / 255.0f);
	}
}

void ColorChooser::onRampChanged()
{
	YASLI_ASSERT(ramp_);
	color_.set(ramp_->get().r, ramp_->get().g, ramp_->get().b, color_.a);
	updateSliders(color_);
	updateEntries(color_);
	updateHex(color_);
	colorRect_->set(color_);
	signalChanged_.emit();
}

void ColorChooser::onSliderChanged()
{
	if(showColor_){
		color_.r = int(sliderRed_->value() * 255);
		color_.g = int(sliderGreen_->value() * 255);
		color_.b = int(sliderBlue_->value() * 255);
	}
	if(showAlpha_){
		color_.a = int(sliderAlpha_->value() * 255);
	}
	if(ramp_)
		ramp_->set(color_);
	colorRect_->set(color_);
	updateEntries(color_);
	updateHex(color_);
	signalChanged_.emit();
}

void ColorChooser::onEntryChanged()
{
	if(showColor_){
		color_.r = clamp(atoi(entryRed_->text()), 0, 255);
		color_.g = clamp(atoi(entryGreen_->text()), 0, 255);
		color_.b = clamp(atoi(entryBlue_->text()), 0, 255);
	}
	if(showAlpha_){
		color_.a = clamp(atoi(entryAlpha_->text()), 0, 255);
	}
	if(ramp_)
		ramp_->set(color_);
	colorRect_->set(color_);
	updateSliders(color_);
	updateHex(color_);
	signalChanged_.emit();
}

void ColorChooser::onHexChanged()
{
	int r = 255, g  = 255, b = 255, a = 255;
	sscanf_s(entryHex_->text(), "%2X%2X%2X%2X", &r, &g, &b, &a);
	color_.set(r, g, b, a);
	colorRect_->set(color_);
	updateEntries(color_);
	updateSliders(color_);
	signalChanged_.emit();
}

}
