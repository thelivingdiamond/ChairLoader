#pragma once 

#include "yasli/Archive.h"

namespace yasli
{

template<class T>
struct RangeDecorator
{
	T* value;
	T softMin;
	T softMax;
	T hardMin;
	T hardMax;

	void YASLI_SERIALIZE_METHOD(Archive& ar) {}
};

template<class T>
RangeDecorator<T> Range(T& value, float hardMin, float hardMax)
{
	RangeDecorator<T> r;
	r.value = &value;
	r.softMin = hardMin;
	r.softMax = hardMax;
	r.hardMin = hardMin;
	r.hardMax = hardMax;
	return r;
}

template<class T>
RangeDecorator<T> Range(T& value, float softMin, float softMax, float hardMin, float hardMax)
{
	RangeDecorator<T> r;
	r.value = &value;
	r.softMin = softMin;
	r.softMax = softMax;
	r.hardMin = hardMin;
	r.hardMax = hardMax;
	return r;
}

template<class T>
bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, RangeDecorator<T>& value, const char* name, const char* label)
{
	if (ar.isEdit())
		return ar(Serializer(value), name, label);
	else
		return ar(*value.value, name, label);
}

}
