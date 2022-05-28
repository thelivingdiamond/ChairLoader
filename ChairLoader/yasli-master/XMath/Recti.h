#pragma once

#include "xmath.h"

class Recti{
public:
    Recti()
    {}
    Recti(Vect2i _min, Vect2i _max)
    : min_(_min), max_(_max) {}
    Recti(int minX, int minY, int maxX, int maxY){
        set(minX, minY, maxX, maxY);
    }
    void set(Vect2i _min, Vect2i _max){
        min_ = _min;
        max_ = _max;
    }
    void set(int minX, int minY, int maxX, int maxY){
        min_.x = minX;
        min_.y = minY;
        max_.x = maxX;
        max_.y = maxY;
    }
	const Vect2i& leftTop() const { return min_; }
	const Vect2i& rightBottom() const { return max_; }
	Vect2i center() const{ 
		return (min_ + max_) / 2; 
	}
    Vect2i size() const{ 
		return max_ - min_; 
	}
    bool isValid() const{
        return min_.x <= max_.x && min_.y <= max_.y;
    }

	Recti operator+(Vect2i offset){
		return Recti( min_ + offset, max_ + offset );
	}

	bool operator==(const Recti& rect) const {
		return min_ == rect.min_ && max_ == rect.max_;
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

    bool pointInside(Vect2i _point) const // -> inside
    {
      if ( _point.x < min_.x || _point.x > max_.x )
        return false;
      if ( _point.y < min_.y || _point.y > max_.y )
        return false;
      return true;
    }

	void addBound(const Vect2i& point)
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

	struct tagRECT& rect() { return (tagRECT&)(*this); }
	const tagRECT& rect() const { return (tagRECT&)(*this); }

	void serialize(yasli::Archive& ar);

private:
	Vect2i min_, max_; 
};
