/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#ifdef WIN32
# ifndef _WIN32_WINNT
# define _WIN32_WINNT 0x0500
# endif
# define WIN32_LEAN_AND_MEAN
#endif

#include <typeinfo>
#include <vector>
#include <list>
#include <map>
#include <string.h>

#ifndef WIN32
# include <stdio.h>
# include <stdlib.h>
#endif

#include "Assert.h"

