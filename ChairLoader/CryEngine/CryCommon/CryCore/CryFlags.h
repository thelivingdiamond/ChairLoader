// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   CryFlags.h
//  Created:     24/01/2012 by Stephen M. North.
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __CryFlags_h__
	#define __CryFlags_h__

	#include <limits> // std::numeric_limits
	#include <CryCore/Assert/CompileTimeAssert.h>

template<typename STORAGE>
class CCryFlags
{
	// the reason for the following assert: AreMultipleFlagsActive() works incorrect if STORAGE is signed
	COMPILE_TIME_ASSERT(!std::numeric_limits<STORAGE>::is_signed);
public:
	CCryFlags() : m_bitStorage(0) {}
	 void    AddFlags(STORAGE flags)                  { m_bitStorage |= flags; }
	 void    ClearFlags(STORAGE flags)                { m_bitStorage &= ~flags; }
	 bool    AreAllFlagsActive(STORAGE flags) const   { return((m_bitStorage & flags) == flags); }
	 bool    AreAnyFlagsActive(STORAGE flags) const   { return((m_bitStorage & flags) != 0); }
	 bool    AreMultipleFlagsActive() const           { return (m_bitStorage & (m_bitStorage - 1)) != 0; }
	 bool    IsOneFlagActive() const                  { return m_bitStorage != 0 && !AreMultipleFlagsActive(); }
	 void    ClearAllFlags()                          { m_bitStorage = 0; }
	 void    SetFlags(STORAGE flags, const bool bSet) { if (bSet) m_bitStorage |= flags; else  m_bitStorage &= ~flags; }
	 STORAGE GetRawFlags() const                      { return m_bitStorage; }
	 bool    operator==(const CCryFlags& other) const { return m_bitStorage == other.m_bitStorage; }
	 bool    operator!=(const CCryFlags& other) const { return !(*this == other); }

private:
	STORAGE m_bitStorage;
};

#endif // __CryFlags_h__
