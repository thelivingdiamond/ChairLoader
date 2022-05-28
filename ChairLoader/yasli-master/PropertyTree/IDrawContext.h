#pragma once

#include "Color.h"
#include "Rect.h"

namespace yasli { struct IconXPM; }

class PropertyTree;

namespace property_tree {

enum Font
{
	FONT_NORMAL,
	FONT_BOLD
};

struct Rect;

enum CheckState {
	CHECK_SET,
	CHECK_NOT_SET,
	CHECK_IN_BETWEEN
};


enum {
	BUTTON_PRESSED = 1 << 0,
	BUTTON_FOCUSED = 1 << 1,
	BUTTON_DISABLED = 1 << 2,
	BUTTON_DROP_DOWN = 1 << 3,
	BUTTON_CENTER_TEXT = 1 << 4
};

struct IDrawContext
{
	const PropertyTree* tree;
	Rect widgetRect;
	Rect lineRect;
	bool captured;
	bool pressed;

	IDrawContext()
	: tree(0)
	, captured(false)
	, pressed(false)
	{
	}

	virtual void drawControlButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font) = 0;
	virtual void drawButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font) = 0;
	virtual void drawCheck(const Rect& rect, bool disabled, CheckState checked) = 0;
	virtual void drawColor(const Rect& rect, const Color& color) = 0;
	virtual void drawComboBox(const Rect& rect, const char* text) = 0;
	virtual void drawEntry(const Rect& rect, const char* text, bool pathEllipsis, bool grayBackground, int trailingOffset) = 0;
	virtual void drawRowLine(const Rect& rect) = 0;
	virtual void drawHorizontalLine(const Rect& rect) = 0;
	virtual void drawIcon(const Rect& rect, const yasli::IconXPM& icon) = 0;
	virtual void drawLabel(const char* text, Font font, const Rect& rect, bool selected) = 0;
	virtual void drawNumberEntry(const char* text, const Rect& rect, bool selected, bool grayed) = 0;
	virtual void drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed) = 0;
	virtual void drawSelection(const Rect& rect, bool inlinedRow) = 0;
	virtual void drawValueText(bool highlighted, const char* text) = 0;
};

}

using namespace property_tree; // temporary
