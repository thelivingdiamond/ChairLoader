/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "XMath/Colors.h"
#include "XMath/Rectf.h"
#include "ww/Rect.h"

struct HDC__;
typedef HDC__* HDC;

namespace ww{

enum TextAlign{
    ALIGN_LEFT = 1,
    ALIGN_CENTER = 2,
    ALIGN_RIGHT = 4,
    ALIGN_VCENTER = 8,
};

enum SysColor{
    SYSCOLOR_HIGHLIGHT,
    SYSCOLOR_BTNFACE,
    SYSCOLOR_BTNSHADOW
};

Color4c sysColor(SysColor color);

class Canvas
{
public:
    Canvas(HDC dc, HWND wnd);
    Canvas() : dc_(0) {}

    void drawText(const Vect2f& pos, const char* text, const Color4c& textColor, float xAlign, float yAlign);
    void drawLine(const Vect2f& start, const Vect2f& end, const Color4c& color, int width);
    void fillRectangle(const Rectf& rect, const Color4c& color);
    void fillCircle(const Vect2f& pos, float radius, const Color4c& color);

    Rectf visibleArea() const;

    Vect2 toView(const Vect2f& p) const;
    Vect2f fromView(const Vect2& p) const;
private:
    HDC dc_;
    Rect visibleRect_;
};

}
