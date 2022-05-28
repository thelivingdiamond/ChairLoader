// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryExtension/ICryUnknown.h>

struct IGameFramework;

struct IGameFrameworkExtensionCreator : public ICryUnknown
{
	CRYINTERFACE_DECLARE(IGameFrameworkExtensionCreator, 0x86197E35AD024DA8, 0xA8D483A98F424FFD);

	//! Creates an extension and returns the interface to it (extension interface must derivate for ICryUnknown).
	//! \param pIGameFramework Pointer to game framework interface, so the new extension can be registered against it.
	//! \param pData Pointer to data needed for creation. Each system interface is responsible for explaining what data is expected to be passed here, if any, 'nullptr' is valid for default extensions.
	//! \return ICryUnknown pointer to just created extension (it can be safely casted with cryinterface_cast< > to the corresponding interface).
	virtual ICryUnknown* Create(IGameFramework* pIGameFramework, void* pData) = 0;
};
DECLARE_SHARED_POINTERS(IGameFrameworkExtensionCreator);
