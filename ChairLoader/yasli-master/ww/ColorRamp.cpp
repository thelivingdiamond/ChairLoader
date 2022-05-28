/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/ColorRamp.h"
#include "ww/Color.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Handle.h"
#include "ww/Win32/Rectangle.h"
#include <float.h>
#include <algorithm>

namespace ww{

struct HBSColor{
	HBSColor(float _hue = 0.0f, float _brightness = 1.0f, float _saturation = 0.0f)
	: hue(_hue)
	, brightness(_brightness)
	, saturation(_saturation)
	{}

	HBSColor(const Color& color);

	void set(const Color& color);

	Color toRGB() const;

	float hue;
	float brightness;
	float saturation;
};


void HBSColor::set(const Color& color)
{
	float r = color.r / 255.0f;
	float g = color.g / 255.0f;
	float b = color.b / 255.0f;

	float minval = std::min(r, std::min(g, b));
    float maxval = std::max(r, std::max(g, b));
    float mdiff = float(maxval) - float(minval);
    float msum = float(maxval) + float(minval);

    brightness = msum / 2.0f;
    if(maxval == minval){
        saturation = 0.0f;
        hue = 0.0f;
    }
    else{
        float rnorm = (maxval - r) / mdiff;      
        float gnorm = (maxval - g) / mdiff;
        float bnorm = (maxval - b) / mdiff;   

        if(r == maxval)
            hue = 60.0f * (6.0f + bnorm - gnorm);
        if(g  == maxval)
            hue = 60.0f * (2.0f + rnorm - bnorm);
        if(b  == maxval)
            hue = 60.0f * (4.0f + gnorm - rnorm);
        if(hue > 360.0f)
            hue = hue - 360.0f;

        saturation = mdiff / maxval;
		brightness = maxval;
	}
}

static float toRGBAux(float rm1, float rm2, float rh)
{
    if(rh > 360.0f)
        rh -= 360.0f;
    else if(rh <   0.0f)
        rh += 360.0f;
   
    if(rh < 60.0f)
        rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
    else if(rh < 180.0f)
        rm1 = rm2;
    else if(rh < 240.0f)
        rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      
                     
    return rm1;
}

Color HBSColor::toRGB() const
{
	Color brithnessColor(int(brightness * 255.0f),
						 int(brightness * 255.0f),
						 int(brightness * 255.0f));
    if(saturation == 0){
		return brithnessColor;
    }
    else{
	  Color result;
	  result.r = int(toRGBAux(0.0f, 1.0f, hue + 120.0f) * brightness* 255.0f);
      result.g = int(toRGBAux(0.0f, 1.0f, hue) * brightness * 255.0f);
      result.b = int(toRGBAux(0.0f, 1.0f, hue - 120.0f) * brightness* 255.0f);
	  result.a = 255;
      
	  return result.interpolate(brithnessColor, 1.0f - saturation);
    }
}


class ColorRampImpl : public _WidgetWindow{
public:
	static const int HUE_WIDTH = 10;
	static const int BORDER_WIDTH = 4;

	ColorRampImpl(ColorRamp* owner);
	void createRampBitmap();
	void handleMouse(POINT point);
	void handleMouseHue(POINT point);

	void redraw(HDC dc);
	void onMessagePaint();
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	void onMessageMouseMove(UINT button, int x, int y);
	BOOL onMessageEraseBkgnd(HDC dc);
	BOOL onMessageSetCursor(HWND window, USHORT hitTest, USHORT message);

	void setColor(const Color& color);
protected:
	Win32::Handle<HBITMAP> rampBitmap_;
	int mouseArea_;
	HBSColor hlsColor_;
	ColorRamp* owner_;
};

ColorRampImpl::ColorRampImpl(ColorRamp* owner)
: _WidgetWindow(owner)
, hlsColor_(0.0f, 0.5f, 1.0f)
, mouseArea_(0)
, owner_(owner)
{
	create(L"", WS_CHILD | WS_TABSTOP, Rect(0, 0, 40, 40), Win32::getDefaultWindowHandle(), WS_EX_CLIENTEDGE);
	createRampBitmap();
}

void ColorRampImpl::setColor(const Color& color)
{
	hlsColor_.set(color);
	if(::IsWindow(handle())){
		createRampBitmap();
        InvalidateRect(handle(), 0, FALSE);
	}
}

bool convertToDFB(HBITMAP& hBitmap)
{
  bool converted = false;
  BITMAP stBitmap;

  if(GetObject(hBitmap, sizeof(stBitmap), &stBitmap) && stBitmap.bmBits){
      // that is a DIB. Now we attempt to create
      // a DFB with the same sizes, and with the pixel
      // format of the display (to omit conversions
      // every time we draw it).
      HDC hScreen = GetDC(NULL);
      YASLI_ASSERT(hScreen);

      HBITMAP hDfb = CreateCompatibleBitmap(hScreen, stBitmap.bmWidth, stBitmap.bmHeight);
      if(hDfb){
          // now let's ensure what we've created is a DIB.
          if(GetObject(hDfb, sizeof(stBitmap), &stBitmap) && !stBitmap.bmBits){
              // ok, we're lucky. Now we have
              // to transfer the image to the DFB.
              HDC hMemSrc = CreateCompatibleDC(NULL);
              if(hMemSrc){
                  HGDIOBJ hOldSrc = SelectObject(hMemSrc, hBitmap);
                  if(hOldSrc){
                      HDC hMemDst = CreateCompatibleDC(NULL);
                      if(hMemDst){
                          HGDIOBJ hOldDst = SelectObject(hMemDst, hDfb);
                          if(hOldDst){
                              if(BitBlt(hMemDst, 0, 0, stBitmap.bmWidth, stBitmap.bmHeight, hMemSrc, 0, 0, SRCCOPY))
                                  converted = true; // success

                              WW_VERIFY(SelectObject(hMemDst, hOldDst));
                          }
                          WW_VERIFY(DeleteDC(hMemDst));
                      }
                      WW_VERIFY(SelectObject(hMemSrc, hOldSrc));
                  }
                  WW_VERIFY(DeleteDC(hMemSrc));
              }
          }

          if (converted){
              WW_VERIFY(DeleteObject(hBitmap)); // it's no longer needed
              hBitmap = hDfb;
          }
          else
              WW_VERIFY(DeleteObject(hDfb));
      }
      ReleaseDC(0, hScreen);
  }
  return converted;
}

void ColorRampImpl::createRampBitmap()
{
    const int width = 256;
    const int height = 256;
    DWORD* pixels = 0;

    HBSColor hlsColor = hlsColor_;
    hlsColor.saturation = 1.0f;
    hlsColor.brightness = 1.0f;
    Color color = hlsColor.toRGB();

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = 0;

	HBITMAP bitmap = CreateDIBSection(::GetDC(0), &bi, DIB_RGB_COLORS, (void**)&pixels, 0, 0);
	if(pixels)
		for(int j = 0; j < height; ++j)
			for(int i = 0; i < width; ++i){
                pixels[j * width + i] = ((((((color.r * j) * i >> 8) + (j * (255 - i))) & 0xFFFFFF00) << 8) +
                                          ((((color.g * j) * i >> 8) + (j * (255 - i))) & 0xFFFFFF00) +
                                         (((((color.b * j) * i >> 8) + (j * (255 - i))) >> 8)));
			}

	if(!bitmap)
		delete[] pixels;

    WW_VERIFY(convertToDFB(bitmap));
	rampBitmap_ = bitmap;
}

void ColorRampImpl::onMessagePaint()
{
	PAINTSTRUCT paintStruct;
	HDC dc = BeginPaint(handle(), &paintStruct);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(handle(), &paintStruct);
}

void ColorRampImpl::redraw(HDC dc)
{
	Win32::Rect rect;
    GetClientRect(handle(), &rect);
    int width = rect.width();
    int height = rect.height();

    HDC tempDC;
    tempDC = CreateCompatibleDC(dc);
    SelectObject(tempDC, rampBitmap_);

	int ramp_width = width - HUE_WIDTH - BORDER_WIDTH;
	int ramp_height = height;

	SetStretchBltMode(dc, HALFTONE);
    StretchBlt(dc, 0, 0, ramp_width, ramp_height, tempDC, 0, 0, 255, 255, SRCCOPY);
    
    // Курсор в Saturation/Brightness области
    int posY = xround((1.0f - hlsColor_.brightness) * float(ramp_height));
    int posX = xround((hlsColor_.saturation) * float(ramp_width));
    Color color = hlsColor_.toRGB();

    FillRect(dc, &Win32::Rect(posX - 4, posY - 4, posX + 5, posY + 5), HBRUSH(GetStockObject(BLACK_BRUSH)));
	HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
    FillRect(dc, &Win32::Rect(posX - 3, posY - 3, posX + 4, posY + 4), brush);
	DeleteObject(brush);

    FillRect(dc, &Win32::Rect(ramp_width, 0, ramp_width + BORDER_WIDTH, height), HBRUSH(GetStockObject(BLACK_BRUSH)));

	HGDIOBJ oldPen = SelectObject(dc, GetStockObject(DC_PEN));
	for(int i = 0; i < rect.height(); ++i){
		float hue = 360.0f * float(i) / float(rect.height());
        Color color = HBSColor(hue, 1.0f, 1.0f).toRGB();
		SetDCPenColor(dc, RGB(color.r, color.g, color.b));
		MoveToEx(dc, width - HUE_WIDTH, i, 0);
		LineTo(dc, width + width - HUE_WIDTH, i);
		//FillSolidRect(dc, width - HUE_WIDTH, i, width, 1, RGB());
	}
	::SelectObject(dc, oldPen);

    // Курсор на Hue полоске
	int pos = xround(hlsColor_.hue / 360.0f * rect.height());
		
	color = HBSColor(hlsColor_.hue, 1.0f, 1.0f).toRGB();
	FillRect(dc, &Win32::Rect(width - HUE_WIDTH - 2, pos - 2, width + 2, pos + 3), HBRUSH(GetStockObject(BLACK_BRUSH)));
	brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
	FillRect(dc, &Win32::Rect(width - HUE_WIDTH - 1, pos - 1, width + 1, pos + 2), brush);
	DeleteObject(brush);

	DeleteDC(tempDC);
}

void ColorRampImpl::handleMouse(POINT point)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
	int cx = rect.width() - BORDER_WIDTH - HUE_WIDTH;
	int cy = rect.height();

    float brightness = clamp(float(cy - point.y) / cy, 0.0f, 1.0f);
    float saturation = clamp(float(point.x) / cx, 0.0f, 1.0f);
	hlsColor_.brightness = brightness;
	hlsColor_.saturation = saturation;
	owner_->color_ = hlsColor_.toRGB();
	owner_->signalChanged_.emit();
	mouseArea_ = 1;
	SetCapture(handle());
	
	RedrawWindow(handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}

void ColorRampImpl::handleMouseHue(POINT point)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
	int cx = rect.width() - BORDER_WIDTH - HUE_WIDTH;
	int cy = rect.height();

	float hue = clamp(float(point.y) * 360.0f / float(cy), 0.0f, 360.0f);
	if(fabs(hlsColor_.hue - hue) > FLT_EPSILON){
		hlsColor_.hue = hue;
		createRampBitmap();
		owner_->color_ = hlsColor_.toRGB();
		owner_->signalChanged_.emit();
		RedrawWindow(handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	mouseArea_ = 0;
	SetCapture(handle());
}

void ColorRampImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
	int cx = rect.width() - BORDER_WIDTH - HUE_WIDTH;
	int cy = rect.height();

	POINT point = {x, y};
	if(point.x > cx)
		handleMouseHue(point);
	else
		handleMouse(point);
	SetFocus(handle());    
	__super::onMessageLButtonDown(button, x, y);
}

void ColorRampImpl::onMessageMouseMove(UINT button, int x, int y)
{
	if(button & MK_LBUTTON && GetCapture() == handle()){
		Win32::Rect rect;
		GetClientRect(handle(), &rect);
		int cx = rect.width() - BORDER_WIDTH - HUE_WIDTH;
		int cy = rect.height();

		POINT point = {x, y};
		if(mouseArea_ == 0)
			handleMouseHue(point);
		else
			handleMouse(point);
	}
	__super::onMessageMouseMove(button, x, y);
}

void ColorRampImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	if(GetCapture() == handle()){
		ReleaseCapture();
	}
	__super::onMessageLButtonUp(button, x, y);
}

BOOL ColorRampImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}

BOOL ColorRampImpl::onMessageSetCursor(HWND window, USHORT hitTest, USHORT message)
{
	HCURSOR cursor = ::LoadCursor(0, MAKEINTRESOURCE(IDC_CROSS));
	::SetCursor(cursor);
	return FALSE;
}

// ---------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
ColorRamp::ColorRamp(int border)
: _WidgetWithWindow(new ColorRampImpl(this), border)
, color_(1.0f, 1.0f, 1.0f, 1.0f)
{

}
#pragma warning(pop)

void ColorRamp::set(const Color& color)
{
	color_ = color;
	impl().setColor(color);
}

ColorRampImpl& ColorRamp::impl()
{
	return *static_cast<ColorRampImpl*>(__super::_window());
}

}
