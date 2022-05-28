#pragma once

#include "xmath.h"
#include <functional>
#include <memory>
#include <algorithm>

#undef min
#undef max


class Rectf{
public:
    Rectf()
    {}
    Rectf(Vect2f _min, Vect2f _max)
    : min_(_min), max_(_max) {}
    Rectf(float minX, float minY, float maxX, float maxY){
        set(minX, minY, maxX, maxY);
    }
    void set(Vect2f _min, Vect2f _max){
        min_ = _min;
        max_ = _max;
    }
    void set(float minX, float minY, float maxX, float maxY){
        min_.x = minX;
        min_.y = minY;
        max_.x = maxX;
        max_.y = maxY;
    }
    bool isValid() const{
        return min_.x <= max_.x && min_.y <= max_.y;
    }
    void validate(){
        if(min_.x > max_.x)
            std::swap(min_.x, max_.x);
        if(min_.y > max_.y)
            std::swap(min_.y, max_.y);
    }
	Rectf operator+(Vect2f offset){
		return Rectf( min_ + offset, max_ + offset );
	}

	Rectf& operator*=(const Vect2f& v){ 
		min_ *= v; max_ *= v; return *this; 
	}
	Rectf operator*(const Vect2f& v) const { 
		return Rectf(min_*v, max_*v); 
	}

	const Vect2f& leftTop() const { return min_; }
	const Vect2f& rightBottom() const { return max_; }
	Vect2f& leftTop() { return min_; }
	Vect2f& rightBottom() { return max_; }
	Vect2f center() const{ return (min_ + max_) / 2; }
	Vect2f size() const{ return max_ - min_; }

    void setLeft( float _left ){ min_.x = _left; }
    void setTop( float _top ){ min_.y = _top; }
    void setRight( float _right ){ max_.x = _right; }
    void setBottom( float _bottom ){ max_.y = _bottom; }
    void setWidth( float _width ){ max_.x = min_.x + _width; }
    void setHeight( float _height ){ max_.y = min_.y + _height; }
	float left() const{ return min_.x; }
	float top() const{ return min_.y; }
	float right() const{ return max_.x; }
	float bottom() const{ return max_.y; }
	float width() const{ return max_.x - min_.x; }
	float height() const{ return max_.y - min_.y; }

    bool pointInside(Vect2f point) const
    {
      if ( point.x < min_.x || point.x > max_.x )
        return false;
      if ( point.y < min_.y || point.y > max_.y )
        return false;
      return true;
    }

    void addBound(const Rectf& rect)
    {
		min_.x = std::min(min_.x, rect.min_.x);
		min_.y = std::min(min_.y, rect.min_.y);
		max_.x = std::max(max_.x, rect.max_.x);
		max_.y = std::max(max_.y, rect.max_.y);
    }

	void serialize(yasli::Archive& ar);

private:
	Vect2f min_;
	Vect2f max_;
};

