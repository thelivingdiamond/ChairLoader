#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#undef min
#undef max

template <class T> T min(const T& x, const T& y){ return x < y ? x : y; }
template <class T> T max(const T& x, const T& y){ return x > y ? x : y; }

template <class T> T min(const T& a, const T& b, const T& c) { return min(min(a, b), c); }
template <class T> T max(const T& a, const T& b, const T& c) { return max(max(a, b), c); }

