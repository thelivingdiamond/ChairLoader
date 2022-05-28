#include "stdafx.h"
#include "Rectf.h"
#include "yasli/Archive.h"
using namespace yasli;

void Rectf::serialize(Archive& ar)
{
  ar( min_.x, "", "^left" );
  ar( min_.y, "", "^top" );
  ar( max_.x, "", "^right" );
  ar( max_.y, "", "^bottom" );
}
