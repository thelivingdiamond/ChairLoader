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
#include "ww/_WidgetWithWindow.h"
#include "ww/Win32/Handle.h"

#include "XMath/XMath.h"
#include "XMath/Colors.h"
#include "XMath/Rectf.h"

namespace ww{
class Viewport2DImpl;
struct KeyPress;

class Viewport2D : public _WidgetWithWindow 
{
public:
	enum TextAlign {
		ALIGN_LEFT = 1,
		ALIGN_CENTER = 2,
		ALIGN_RIGHT = 4,
		ALIGN_VCENTER = 8,
	};

	explicit Viewport2D(int border, int fontHeight);
	~Viewport2D();

    void serialize(Archive& ar);

	// updateNow влияет на то, положится ли сообщение перерисовки в очередь сообщений
	// (на потом) или вызовется непосредственно сейчас
	void redraw(bool updateNow = false);

	// для получения координат используется mousePosition()
	virtual void onMouseMove(const Vect2& delta);
	virtual void onMouseButtonDown(MouseButton button);
	virtual void onMouseButtonUp(MouseButton button);

	virtual void onKeyDown(const KeyPress& key) {}
	virtual void onKeyUp(const KeyPress& key) {}

	virtual void onRedraw(HDC dc);
	virtual void onResize(int width, int height);

	void captureMouse();
	void releaseMouse();

	Vect2 size() const{ return size_; }
	Vect2 mousePosition() const{ return mousePosition_; }
	
	void setBackgroundColor(Color3c backgroundColor){
		backgroundColor_ = backgroundColor;
		redraw();
	}
	Color3c backgroundColor() const{ return backgroundColor_; }

	void drawPixel(HDC dc, const Vect2f& pos, const Color4c& color);
	void drawCircle(HDC dc, const Vect2f& pos, float radius, const Color4c& color, int outline_width);
	void drawLine(HDC dc, const Vect2f& start, const Vect2f& end, const Color4c& color, int style = PS_SOLID, int width = 1);
	void drawRectangle(HDC dc, const Rectf& rect, const Color4c& color);
	void fillRectangle(HDC dc, const Rectf& rect, const Color4c& color);
	void drawText(HDC dc, const Rectf& rect, const char* text, TextAlign align = ALIGN_CENTER, bool endEllipsis = false);
	void drawText(HDC dc, const Vect2f& pos, const char* text, const Color4c& textColor, const Color4c& backColor = Color4c(0, 0, 0, 255), int align = ALIGN_CENTER);

    void setViewCenter(const Vect2f& center);
	const Vect2f& viewScale() const { return viewScale_; }
	float pixelWidth() const { return 1.f/fabsf(viewScale_.x); }
	float pixelHeight() const { return 1.f/fabsf(viewScale_.y); }
	Vect2 coordsToScreen(const Vect2f& pos) const;
	Vect2f coordsFromScreen(const Vect2& pos) const;
	void centerOn(const Vect2f& point) { viewOffset_ = -point; }
	Rectf visibleArea() const;
	void setVisibleArea(const Rectf& rect, bool preserveAspect = true);
    void enableBasicNavigation(bool enable);

	bool lmbPressed() const { return lmbPressed_; }
	bool rmbPressed() const { return rmbPressed_; }
	bool scrolling() const { return scrolling_; }

	Vect2f clickPoint() const { return coordsFromScreen(mousePosition()); }

protected:
	int fontHeight_;
	Win32::Handle<HFONT> positionFont_;

    bool enableBasicNavigation_;
    Vect2f viewCenter_;

	Vect2 viewSize_;
	Vect2f viewOffset_;
	Vect2f viewScale_;
	int zoomIndex_;

	Color3c backgroundColor_;

	bool lmbPressed_;
	bool rmbPressed_;
	bool scrolling_;

	Vect2 mousePosition_;
	Vect2 size_;

	void createFont();

	Viewport2DImpl* impl();
	friend class Viewport2DImpl;
};

}

