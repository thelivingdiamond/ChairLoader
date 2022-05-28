// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "Target.h"
#include <CrySerialization/Math.h>

CRY_PFX2_DBG

volatile bool gPfx2Target = false;

namespace pfx2
{

SERIALIZATION_ENUM_IMPLEMENT(ETargetSource);

CTargetSource::CTargetSource(ETargetSource source)
	: m_offset(ZERO)
	, m_source(source)
{
}

void CTargetSource::Serialize(Serialization::IArchive& ar)
{
	ar(m_source, "Source", "^");
	ar(m_offset, "Offset", "Offset");
}

}
