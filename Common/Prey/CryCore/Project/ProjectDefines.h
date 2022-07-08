#pragma once

#include <Prey/CryCore/BaseTypes.h>

// Uncomment one of the two following typedefs:
typedef uint32 vtx_idx;
//typedef uint16 vtx_idx;

// Don't override malloc/free/etc to point to CryEngine allocator
#define NO_CRY_MALLOC_OVERRIDE 1

// Enable cvar whitelist so it can be disabled by Chairloader's dev console
#define CVARS_WHITELIST 1