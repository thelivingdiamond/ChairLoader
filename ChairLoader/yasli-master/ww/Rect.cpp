/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Rect.h"
#include "yasli/Archive.h"

namespace ww
{

void Rect::serialize(yasli::Archive& ar)
{
  ar( min_.x, "", "&minX" );
  ar( min_.y, "", "&minY" );
  ar( max_.x, "", "&maxX" );
  ar( max_.y, "", "&maxY" );
}

}
