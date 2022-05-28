#include "StdAfx.h"
#include "XMath/Colors.h"
#include "ComboListColor.h"

void ComboListColor::serialize(yasli::Archive& ar)
{
	ar(index_, "index", 0);
}

