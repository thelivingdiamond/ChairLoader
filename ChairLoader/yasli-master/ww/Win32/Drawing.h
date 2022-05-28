/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Win32/Types.h"
#include "ww/Rect.h"

class KeysColor;
struct Color4f;
struct Color4c;

namespace Gdiplus{
	class Graphics;
	class Rect;
	class Brush;
	class Color;
	class Font;
}

namespace Win32{
	using yasli::Archive;

	COLORREF toColorRef(const Color4f& color);
	Color4c toColor4c(COLORREF c);
	COLORREF blendColor(COLORREF from, COLORREF to, float factor);
	// void drawGradient(HDC dc, const RECT& rect, KeysColor& gradient, bool alphaOnly = false);
	void drawColorBlend(HDC dc, const ww::Rect& rect, const Color4f& color1, const Color4f& color2, bool alphaOnly = false);

	void drawVerticalBlend(HDC dc, const RECT& rect, COLORREF color1, COLORREF color2);
	void drawGrayedCheck(HDC dc, const RECT& iconRect);
    void drawRadio(Gdiplus::Graphics *gr, const Gdiplus::Rect& checkRect, bool checked);
	void drawNotCheck(Gdiplus::Graphics* gr, const Gdiplus::Rect& rect, bool checked);
	void drawEdit(HDC dc, const RECT& rect, const wchar_t* text, HFONT font, bool pathEllipsis = false, bool grayBackground = false);
	void drawButton(HDC dc, const RECT& rect, const wchar_t* text, HFONT font);
	void drawComboBox(HDC dc, const RECT& rect, const wchar_t* text, HFONT font);
	void drawSlider(HDC dc, const RECT& rect, float value, bool focused);
	bool isAppThemed();
}

namespace ww{
	void fillRoundRectangle(Gdiplus::Graphics* gr, Gdiplus::Brush* brush, const Gdiplus::Rect& r, const Gdiplus::Color& border, int radius);
}

