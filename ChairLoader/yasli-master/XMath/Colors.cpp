#include "stdafx.h"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#pragma warning(disable : 4073 ) // initializers put in library initialization area
#pragma init_seg(lib)
#endif

#include <algorithm>
using std::min;
using std::max;
#include "Colors.h"
#include "yasli/Archive.h"
#ifdef XMATH_USE_WW_COLOR
#include "ww/Color.h"	
#endif
using namespace yasli;


const Color4f Color4f::WHITE(1, 1, 1, 1);
const Color4f Color4f::BLACK(0, 0, 0, 1);
const Color4f Color4f::RED(1, 0, 0, 1);
const Color4f Color4f::GREEN(0, 1, 0, 1);
const Color4f Color4f::BLUE(0, 0, 1, 1);
const Color4f Color4f::YELLOW(1, 1, 0, 1);
const Color4f Color4f::MAGENTA(1, 0, 1, 1);
const Color4f Color4f::CYAN(0, 1, 1, 1);
const Color4f Color4f::ZERO(0, 0, 0, 0);

const Color4c Color4c::WHITE(255, 255, 255);
const Color4c Color4c::BLACK(0, 0, 0);
const Color4c Color4c::RED(255, 0, 0);
const Color4c Color4c::GREEN(0, 255, 0);
const Color4c Color4c::BLUE(0, 0, 255);
const Color4c Color4c::YELLOW(255, 255, 0);
const Color4c Color4c::MAGENTA(255, 0, 255);
const Color4c Color4c::CYAN(0, 255, 255);
const Color4c Color4c::ZERO(0, 0, 0, 0);

Color4c& Color4c::setGDI(unsigned int color)
{
	r = color & 0xff;
	g = (color >> 8) & 0xff;
	b = (color >> 16) & 0xff;
	a = 255;
	return *this;
}

struct SerializeableColor4c : Color4c {
    void serialize(yasli::Archive& ar) {
        ar(r, "", "&r");
        ar(g, "", "&g");
        ar(b, "", "&b");
        ar(a, "", "&a");
    }
};
bool serialize(yasli::Archive& ar, Color4c& c, const char* name, const char* label) 
{
#ifdef XMATH_USE_WW_COLOR
	if(ar.isEdit()){
		ww::Color wc(c.r, c.g, c.b, c.a);
		bool result = ar(wc, name, label);
		c.set(wc.r, wc.g, wc.b, wc.a);
		return result;
	}
#endif
    return ar((SerializeableColor4c&)c, name, label);
}

struct SerializeableColor4f : Color4f {
    void serialize(yasli::Archive& ar) {
        ar(r, "", "&r");
        ar(g, "", "&g");
        ar(b, "", "&b");
        ar(a, "", "&a");
    }
};
bool serialize(yasli::Archive& ar, Color4f& c, const char* name, const char* label)
{
#ifdef XMATH_USE_WW_COLOR
	if(ar.isEdit()){
		ww::Color wc(c.GetR(), c.GetG(), c.GetB(), c.GetA());
		bool result = ar(wc, name, label);
		c = Color4f(Color4c(wc.r, wc.g, wc.b, wc.a));
		return result;
	}
#endif
    return ar((SerializeableColor4f&)c, name, label);
}

struct SerializeableColor3c : Color3c {
    void serialize(yasli::Archive& ar) {
        ar(r, "", "&r");
        ar(g, "", "&g");
        ar(b, "", "&b");
    }
};
bool serialize(yasli::Archive& ar, Color3c& c, const char* name, const char* label)
{
#ifdef XMATH_USE_WW_COLOR
	if(ar.isEdit()){
		ww::Color wc(c.r, c.g, c.b);
		bool result = ar(wc, name, label);
		c.set(wc.r, wc.g, wc.b);
		return result;
	}
#endif
    return ar((SerializeableColor3c&)c, name, label);
}


// HSV
//Y = 0.30*R + 0.59*G + 0.11*B перевод цветного в чёрно-белый
//h=0..360,s=0..1,v=0..1

inline void HSVtoRGB(float h,float s,float v,
					 float& r,float& g,float& b)
{
	const float min=1e-5f;
	int i;
	float f,m,n,k;

	if(s<min){
		r=g=b=v;
	}
	else {
		if(h>=360.0f)
			h=0;
		else
			h=h/60.0f;

		i=xround(floor(h));
		f=h-i;
		m=v*(1-s);
		n=v*(1-s*f);
		k=v*(1-s*(1-f));

		switch(i){
		case 0:
			r=v; g=k; b=m;
			break;
		case 1:
			r=n; g=v; b=m;
			break;
		case 2:
			r=m; g=v; b=k;
			break;
		case 3:
			r=m; g=n; b=v;
			break;
		case 4:
			r=k; g=m; b=v;
			break;
		case 5:
			r=v; g=m; b=n;
			break;
		default:
			YASLI_ASSERT(0);
		}
	}

	YASLI_ASSERT(r>=0 && r<=1);
	YASLI_ASSERT(g>=0 && g<=1);
	YASLI_ASSERT(b>=0 && b<=1);
}

void Color4f::setHSV(float h,float s,float v, float alpha)
{
	HSVtoRGB(h,s,v, r,g,b);
	a = alpha;
}

void Color4c::setHSV(float h,float s,float v, unsigned char alpha)
{
	float rf,gf,bf;
	HSVtoRGB(h,s,v, rf,gf,bf);
	r = xround(rf*255);
	g = xround(gf*255);
	b = xround(bf*255);
	a = alpha;
}

void Color4c::hsv(float& h,float& s,float& v)
{
	float rf = r/255.f;
	float gf = g/255.f;
	float bf = b/255.f;
	v = max(max(rf,gf),bf);
	float temp=min(min(rf,gf),bf);
	if(v==0)
		s=0;
	else 
		s=(v-temp)/v;

	if(s==0)
		h=0;
	else {
		float Cr=(v-rf)/(v-temp);
		float Cg=(v-gf)/(v-temp);
		float Cb=(v-bf)/(v-temp);

		if(rf==v) {
			h=Cb-Cg;
		}
		else if(gf==v) {
			h=2+Cr-Cb;
		} 
		else if(bf==v) {
			h=4+Cg-Cr;
		}

		h=60*h;
		if(h<0)h+=360;
	}
}
