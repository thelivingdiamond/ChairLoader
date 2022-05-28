/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif
#define NOMINMAX

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#ifdef WIN32
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#endif

#include <string>
#include <vector>
#include <list>
#include "Macros.h"
#include "yasli/Pointers.h"

#pragma warning(disable: 4355) //  'this' : used in base member initializer list

