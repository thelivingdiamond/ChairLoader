// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryEntitySystem/IEntityClass.h>

struct SScriptFuncHandle;

// Header: MadeUp
// _unknown/SEntityScriptEvent.h
struct SEntityScriptEvent // Id=800C775 Size=24
{
	string name;
	SScriptFuncHandle *func;
	IEntityClass::EventValueType valueType;
	uint8_t bOutput : 1;
	uint8_t bOldEvent : 1;
};

