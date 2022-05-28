/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace ww{

class SliderDecoratorf{
public:
	explicit SliderDecoratorf(double value)
    : value_(float(value))
	, valuePointer_(0)
	, minValue_(0.0f)
	, maxValue_(1.0f)
	, step_(1.0f)
	{
	}

	SliderDecoratorf(float value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, minValue_(0.0f)
	, maxValue_(1.0f)
	, step_(1.0f)
	{}

	SliderDecoratorf(float& value, float _range_min, float _range_max, float _step = 0.f)
	: value_(value)
	, valuePointer_(&value)
	, minValue_(_range_min)
	, maxValue_(_range_max)
	, step_(_step)
	{}
	~SliderDecoratorf(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator float() const{
		return value_;
	}
	SliderDecoratorf& operator=(const SliderDecoratorf& rhs){
		value_ = rhs.value_;
		if (rhs.valuePointer_) {
			minValue_ = rhs.minValue_;
			maxValue_ = rhs.maxValue_;
			step_ = rhs.step_;
		}
		return *this;
	}
	SliderDecoratorf& operator=(float value){
		value_ = value;
		return *this;
	}

	float& value() { return value_; }
	const float& value() const { return value_; }

	float step() const { return step_; }
	void clip();
	float minValue() const{ return minValue_; }
	float maxValue() const{ return maxValue_; }

	void serialize(yasli::Archive& ar);

	float minValue_;
	float maxValue_;
	float step_;
	float* valuePointer_;
	float value_;
};



class SliderDecoratori{
public:
	SliderDecoratori(int value = 0.0f)
	: value_(value)
	, valuePointer_(0)
	, minValue_(0)
	, maxValue_(100)
	, step_(10)
	{}
	explicit SliderDecoratori(double value)
	: value_(int(value))
	, valuePointer_(0)
	, minValue_(0)
	, maxValue_(100)
	, step_(1)
	{}

	SliderDecoratori(const SliderDecoratori& original)
	: value_(original.value_)
	, valuePointer_(0)
	, minValue_(original.minValue_)
	, maxValue_(original.maxValue_)
	, step_(original.step_)
	{
	}

	SliderDecoratori(int& value, int _range_min, int _range_max, int _step = 1)
	: value_(value)
	, valuePointer_(&value)
	, minValue_(_range_min)
	, maxValue_(_range_max)
	, step_(_step)
	{}
	~SliderDecoratori(){
		if(valuePointer_)
			*valuePointer_ = value_;
	}

	operator int() const{
		return value_;
	}
	SliderDecoratori& operator=(const SliderDecoratori& rhs){
		value_ = rhs.value_;
		return *this;
	}
	SliderDecoratori& operator=(int value){
		value_ = value;
		return *this;
	}

	int& value() { return value_; }
	const int& value() const { return value_; }

	int step() const { return step_; }
	int minValue() const{ return minValue_; }
	int maxValue() const{ return maxValue_; }
	void clip();

	void serialize(yasli::Archive& ar);

	int minValue_;
	int maxValue_;
	int step_;
	int* valuePointer_;
	int value_;
};

bool serialize(yasli::Archive& ar, ww::SliderDecoratorf &wrapper, const char* name, const char* label);
bool serialize(yasli::Archive& ar, ww::SliderDecoratori &wrapper, const char* name, const char* label);

}

