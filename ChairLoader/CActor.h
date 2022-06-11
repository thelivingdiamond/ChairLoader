#pragma once
#include "IActor.h"

class CActor : IActor
{
public:
	virtual CCryActor* ToCryActor(CActor* this) = 0;
};
