/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/PropertyTree.h"
#include "PropertyTree/Rect.h"
#include "PropertyTree/IDrawContext.h"
#include <map>

class PropertyTree;

namespace Gdiplus{
	class Color;
    class Graphics;
    class Brush;
    class Rect;
    class Font;
	class GraphicsPath;
	class Bitmap;
}

namespace ww{
	struct Color;
	class Icon;
}

namespace property_tree {

struct DrawingCache
{
	HANDLE theme_;

	void initialize();
	void finalize();
	void flush();

	~DrawingCache();

	static DrawingCache* get();

	Gdiplus::Bitmap* getBitmapForIcon(const ww::Icon& icon);
private:
	struct BitmapCache {
		std::vector<ww::Color> pixels;
		Gdiplus::Bitmap* bitmap;
	};
	typedef std::map<ww::Icon, BitmapCache> IconToBitmap;
	IconToBitmap iconToBitmapMap_;

	ULONG_PTR token_;
	int users_;
};


void createRoundRectanglePath(Gdiplus::GraphicsPath* path, const Gdiplus::Rect &_rect, int roundness);
void fillRoundRectangle(Gdiplus::Graphics* gr, Gdiplus::Brush* brush, const Gdiplus::Rect& r, const Gdiplus::Color& borderColor, int radius);
void drawRoundRectangle(Gdiplus::Graphics* graphics, const Gdiplus::Rect &_r, unsigned int color, int radius, int width);
void drawEdit(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, const wchar_t* text, const Gdiplus::Font* font, bool pathEllipsis, bool grayBackground);

Gdiplus::Font* propertyTreeDefaultFont();
Gdiplus::Font* propertyTreeDefaultBoldFont();

class wwDrawContext : public IDrawContext {
public:
	wwDrawContext(ww::PropertyTree* tree, Gdiplus::Graphics* graphics)
	: tree_(tree)
	, graphics(graphics)
	{
		this->tree = tree;
	}

	virtual void drawControlButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font) override;
	virtual void drawButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font) override;
	virtual void drawCheck(const Rect& rect, bool disabled, CheckState checked) override;
	virtual void drawColor(const Rect& rect, const Color& color) override;
	virtual void drawComboBox(const Rect& rect, const char* text) override;
	virtual void drawEntry(const Rect& rect, const char* text, bool pathEllipsis, bool grayBackground, int trailingOffset) override;
	virtual void drawRowLine(const Rect& rect) override;
	virtual void drawHorizontalLine(const Rect& rect) override;
	virtual void drawIcon(const Rect& rect, const yasli::IconXPM& icon) override;
	virtual void drawLabel(const char* text, Font font, const Rect& rect, bool selected) override;
	virtual void drawNumberEntry(const char* text, const Rect& rect, bool selected, bool grayed) override;
	virtual void drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed) override;
	virtual void drawSelection(const Rect& rect, bool inlinedRow) override;
	virtual void drawValueText(bool highlighted, const char* text) override;

	Gdiplus::Graphics* graphics;
private:
	Gdiplus::Font* convertFont(Font font);

	ww::PropertyTree* tree_;
};

}


