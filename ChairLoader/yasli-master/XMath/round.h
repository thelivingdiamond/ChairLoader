#pragma once
#include <math.h>
#ifdef _WIN32
# include <emmintrin.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable : 4244)
#endif

template<class T, class T1, class T2> 
inline T clamp(const T& x, const T1& xmin, const T2& xmax)
{
  if(x < xmin)
    return xmin;
  if(x > xmax)
    return xmax;
  return x;
}

#if defined(_WIN32) 
inline int xround(double x)
{
#ifdef _M_AMD64
	return _mm_cvtsd_si32(_mm_load_sd(&x));
#else
	int a;
	_asm {
        fld x
        fistp dword ptr a
    }
	return a;
#endif
}

inline int xround(float x)
{
	return _mm_cvtss_si32(_mm_load_ss(&x));//assume that current rounding mode is always correct (i.e. round to nearest)
}

//#if _MSC_VER <= 1600
//#define round xround
//#endif

#else // _WIN32

inline int xround(float x)
{
  return lroundf(x);
}

#endif
