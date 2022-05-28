/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Assert.h"

namespace ww{

#pragma warning (push)
#pragma warning (disable:4127)

template<class _Dest, class _Source>
_Dest safe_cast(_Source* source){
#ifdef _DEBUG
	_Dest result = dynamic_cast<_Dest>(source);
	YASLI_ASSERT(!source || result && "safe_cast failed!");
	return result;
#else
	return static_cast<_Dest>(source);
#endif
}

template<class _Dest, class _Source>
_Dest safe_cast(_Source& source){
#ifdef _DEBUG
	return dynamic_cast<_Dest>(source);
#else
	return static_cast<_Dest>(source);
#endif
}

#pragma warning (pop)


}
