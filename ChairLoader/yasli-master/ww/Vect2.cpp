/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Vect2.h"
#include "yasli/Archive.h"

namespace ww {

const Vect2 Vect2::ZERO;

void Vect2::serialize(yasli::Archive& ar)
{
	ar(x, "", "^x");
	ar(y, "", "^y");
}

}
