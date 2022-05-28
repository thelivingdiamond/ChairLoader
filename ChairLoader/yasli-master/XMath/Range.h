#pragma once

#include "XMath/xmath.h"
namespace yasli{ class Archive; }

class Rangef
{
public:
	Rangef(float _min = 0.f, float _max = 0.f)
	: min_(_min)
	, max_(_max)
	{}

	float minimum() const { return min_; }
	void setMinimum(float _min) { min_ = _min; }
	
	float maximum() const { return max_; }
	void setMaximum(float _max) { max_ = _max; }
	
	void set(float _min, float _max);

	float length() const { return max_ - min_; }
	float center() const { return (max_ + min_) / 2.f; }

	bool isValid() const { return min_ <= max_; }

	bool include(float _value) const { return (min_ <= _value) && (max_ >= _value);	}
	bool include(const Rangef& _range) const { return min_ <= _range.min_ && max_ >= _range.max_; }
	
	Rangef intersection(const Rangef& _range) const;
	Rangef merge(const Rangef& _range) const;

	float clip(float& _value) const;

	void serialize(yasli::Archive& ar);

private:
	float min_;
	float max_;
};

// --------------------- Rangei

class Rangei
{
public:
	Rangei(int _min = 0.f, int _max = 0.f)
	: min_(_min)
	, max_(_max)
	{}

	int minimum() const { return min_; }
	void setMinimum(int _min) { min_ = _min; }

	int maximum() const { return max_; }
	void setMaximum(int _max) { max_ = _max; }

	void set(int _min, int _max);

	int length() const { return max_ - min_; }
	int center() const { return (max_ + min_) / 2; }

	bool isValid() const { return min_ <= max_; }

	bool include(int _value) const { return (min_ <= _value) && (max_ >= _value);	}
	bool include(const Rangei& _range) const { return min_ <= _range.min_ && max_ >= _range.max_; }

	Rangei intersection(const Rangei& _range) const;
	Rangei merge(const Rangei& _range) const;

	int clip(int& _value);

	void serialize(yasli::Archive& ar);

private:
	int min_;
	int max_;
};

