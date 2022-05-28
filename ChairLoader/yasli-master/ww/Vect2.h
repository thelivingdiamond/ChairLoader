/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <math.h>

namespace yasli { class Archive; }

namespace ww {

struct Vect2
{
	int x, y;

	Vect2() : x(0), y(0) {}
	Vect2(int x_, int y_) { x = x_; y = y_; }

	void set(int x_, int y_) { x = x_; y=y_; }
	Vect2 operator- () const { return Vect2(-x, -y); }

	const int& operator[](int i) const { return *(&x + i); }
	int& operator[](int i) { return *(&x + i); }

	Vect2& operator+=(const Vect2& v) { x += v.x; y += v.y; return *this; }
	Vect2& operator-=(const Vect2& v) { x -= v.x; y -= v.y; return *this; }
	Vect2& operator*=(const Vect2& v) { x *= v.x; y *= v.y; return *this; }
	Vect2& operator/=(const Vect2& v) { x /= v.x; y /= v.y; return *this; }

	Vect2 operator+(const Vect2& v) const { return Vect2(*this) += v; }
	Vect2 operator-(const Vect2& v) const { return Vect2(*this) -= v; }
	Vect2 operator*(const Vect2& v) const { return Vect2(*this) *= v; }

	Vect2& operator*=(int f) { x *= f; y *= f; return *this; }
	Vect2 operator*(int f) const { return Vect2(*this) *= f; }

	Vect2& operator>>=(int n) { x >>= n; y >>= n; return *this; }
	Vect2 operator>>(int n) const { return Vect2(*this) >>= n; }

	Vect2& operator/=(int f) { x /= f; y /= f; return *this; }
	Vect2 operator/(int f) const { return Vect2(*this) /= f; }
	Vect2 operator/(const Vect2& v) const { return Vect2(*this) /= v; }

	int dot(const Vect2& v) const { return x*v.x + y*v.y; }

	int operator%(const Vect2 &v) const { return x*v.y - y*v.x; }

	int length() const { return int(sqrtf(float(x*x+y*y))); }
	int length2() const { return x*x+y*y; }

	void normalize(int length)
	{
		float f = (float)length / sqrtf((float)(x*x + y*y));
		x = int(x * f);
		y = int(y * f);
	}
	int distance2(const Vect2& v) const
	{ 
		int a = x - v.x; 
		int b = y - v.y;
		return a * a + b * b;
	}
	Vect2& interpolate(const Vect2& u, const Vect2& v, float lambda);

	bool operator==(const Vect2& v)	const { return x == v.x && y == v.y; }
	bool operator!=(const Vect2& v)	const { return x != v.x || y != v.y; }

	void swap(Vect2 &v) { Vect2 tmp = v; v = *this; *this = tmp; }

	void serialize(yasli::Archive& ar);

	static const Vect2 ZERO;
};

}
