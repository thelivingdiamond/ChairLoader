/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Vect2.h"

struct tagRECT; // Win32 RECT

namespace ww {

class Rect
{
public:
    Rect()
    {}
    Rect(Vect2 _min, Vect2 _max)
    : min_(_min), max_(_max) {}
    Rect(int left, int top, int right, int bottom){
        set(left, top, right, bottom);
    }
    void set(Vect2 _min, Vect2 _max){
        min_ = _min;
        max_ = _max;
    }
    void set(int minX, int minY, int maxX, int maxY){
        min_.x = minX;
        min_.y = minY;
        max_.x = maxX;
        max_.y = maxY;
    }
	const Vect2& leftTop() const { return min_; }
	const Vect2& rightBottom() const { return max_; }
	Vect2 center() const{ 
		return (min_ + max_) / 2; 
	}
    Vect2 size() const{ 
		return max_ - min_; 
	}
    bool isValid() const{
        return min_.x <= max_.x && min_.y <= max_.y;
    }

	Rect operator+(Vect2 offset){
		return Rect( min_ + offset, max_ + offset );
	}
	
    void setLeft( int _left ){ min_.x = _left; }
    void setTop( int _top ){ min_.y = _top; }
    void setRight( int _right ){ max_.x = _right; }
    void setBottom( int _bottom ){ max_.y = _bottom; }
    void setWidth( int _width ){ max_.x = min_.x + _width; }
    void setHeight( int _height ){ max_.y = min_.y + _height; }
	int left() const{ return min_.x; }
	int top() const{ return min_.y; }
	int right() const{ return max_.x; }
	int bottom() const{ return max_.y; }
	int width() const{ return max_.x - min_.x; }
	int height() const{ return max_.y - min_.y; }

    bool pointInside(Vect2 _point) const // -> inside
    {
      if ( _point.x < min_.x || _point.x > max_.x )
        return false;
      if ( _point.y < min_.y || _point.y > max_.y )
        return false;
      return true;
    }

	void addBound(const Vect2& point)
	{
		if(min_.x > point.x)
			min_.x = point.x;
		if(min_.y > point.y)
			min_.y = point.y;
		if(max_.x < point.x)
			max_.x = point.x;
		if(max_.y < point.y)
			max_.y = point.y;
	}

	operator tagRECT&() { return (tagRECT&)(*this); }
	operator const tagRECT&() const { return (tagRECT&)(*this); }

	void serialize(yasli::Archive& ar);

private:
	Vect2 min_, max_; 
};

}
