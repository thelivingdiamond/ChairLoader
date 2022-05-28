#include "stdafx.h"
#include "Recti.h"
#include "yasli/Archive.h"
using namespace yasli;

void Recti::serialize(Archive& ar)
{
  ar( min_.x, "", "&minX" );
  ar( min_.y, "", "&minY" );
  ar( max_.x, "", "&maxX" );
  ar( max_.y, "", "&maxY" );
}

