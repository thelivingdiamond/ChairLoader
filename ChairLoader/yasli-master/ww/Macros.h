 /**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Assert.h"
#include "XMath/round.h"

#ifdef _WIN64
#include <emmintrin.h>
#endif

#define FOR_EACH(list, iterator, ...) for(__VA_ARGS__ iterator = (list).begin(); iterator != (list).end(); ++iterator)


template<class T, size_t Len>
char (&globalArrayLenHelper(const T(&)[Len]))[Len];
#define ARRAY_LEN(arr) sizeof(globalArrayLenHelper(arr)) 

#ifndef NDEBUG
# define WW_VERIFY(x) YASLI_ASSERT(x)
#else
# define WW_VERIFY(x) (x)
#endif

#undef min
#undef max

namespace ww {

inline int min(int x,int y){ return x < y ? x : y; }
inline float min(float x,float y){ return x < y ? x : y; }
inline double min(double x,double y){ return x < y ? x : y; }

inline int max(int x,int y){ return x > y ? x : y; }
inline float max(float x,float y){ return x > y ? x : y; }
inline double max(double x,double y){ return x > y ? x : y; }

#pragma warning (disable : 4244)

template<class T, class T1, class T2> 
inline T clamp(const T& x, const T1& xmin, const T2& xmax)
{
  if(x < xmin)
    return xmin;
  if(x > xmax)
    return xmax;
  return x;
}

}
