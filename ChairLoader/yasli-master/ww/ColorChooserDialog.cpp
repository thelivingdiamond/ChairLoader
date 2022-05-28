/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/ColorChooserDialog.h"
#include "ww/ColorChooser.h"
#include "yasli/Assert.h"
#include "yasli/BinArchive.h"

namespace ww{

static string stateFileName = string(getenv("TEMP")) + "\\colorChooserDialog.tmp";
static const int DIALOG_BORDER = 12;

ColorChooserDialog::ColorChooserDialog(ww::Widget* parent, const Color& color, bool showColor, bool showAlpha)
: Dialog(parent, DIALOG_BORDER )
{
	initialize(color, showColor, showAlpha);
}

ColorChooserDialog::ColorChooserDialog(HWND parent, const Color& color, bool showColor, bool showAlpha)
	: Dialog(parent, DIALOG_BORDER)
{
	initialize(color, showColor, showAlpha);
}

void ColorChooserDialog::initialize(const Color& color, bool showColor, bool showAlpha)
{
	YASLI_ASSERT(showColor || showAlpha);

	if (showColor)
		setTitle("Choose Color");
	else
		setTitle("Choose Alpha");

	if (showColor)
		setDefaultSize(350, 400);
	else
		setDefaultSize(400, 0);
	setResizeable(true);

	chooser_ = new ColorChooser();
	chooser_->signalChanged().connect(this, &ColorChooserDialog::onChooserChanged);
	chooser_->setShowColor(showColor);
	chooser_->setShowAlpha(showAlpha);
	add(chooser_, PACK_FILL);

	addButton("OK", RESPONSE_OK);
	addButton("Cancel", RESPONSE_CANCEL);

	set(color);

	BinIArchive ia;
	if (ia.load(stateFileName.c_str())){
		ia.setFilter(SERIALIZE_STATE);
		ia(*this, "window", 0);
	}
}

ColorChooserDialog::~ColorChooserDialog()
{
	BinOArchive oa;
	oa.setFilter(SERIALIZE_STATE);
	oa(*this, "window", 0);
	oa.save(stateFileName.c_str());
}

void ColorChooserDialog::set(const Color& color)
{
	color_ = color;
	chooser_->set(color);
}

void ColorChooserDialog::onChooserChanged()
{
	color_ = chooser_->get();
}

}
