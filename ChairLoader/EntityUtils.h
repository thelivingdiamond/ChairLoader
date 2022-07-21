#pragma once
#include <Prey/ArkEntityArchetypeLibrary.h>
#include <Prey/ArkEntityClassLibrary.h>
#include <Prey/ArkAbilityLibrary.h>
#include "pch.h"

#include "preyDllObjects.h"
#include "PreyFunctions.h"
#include <Prey/ArkPlayer.h>


// spawn non-npc entities through IEntitySystem.spawnNpcfromArchetype()
   //- create spawn parameters from Vec3 pos, Rot, name, archetype ID or class ID
// spawn npc entities
   //- create/manage a chairloader ArkNpcSpawner 
   //- set the archetype of the spawner
   //- spawn the npcs by requesting a spawn
   //- 

class EntityUtils {
public:
	void DumpEntity(CEntity* entity, bool dumpProxies);
	void DumpGameObject(CGameObject* obj);

	IEntity* spawnNpcFromArchetype(const char* name, Vec3& pos, Quat& rot, uint64 archetypeId, unsigned int spawnCount = 1);
	IEntity* spawnEntityFromArchetype(const char* name, Vec3 pos, Quat rot, uint64 archetypeId, unsigned int spawnCount = 1);

	// entities (WIP)
	// CEntity* CreateEntity(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, PreyFunctions* functions);
	// CEntity* CreateEntityBasic(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, PreyFunctions* functions);
	SEntitySpawnParams* CreateEntitySpawnParameters(const char* name, Vec3 pos, Quat rot, SEntitySpawnParams* params, IEntityArchetype* entityArchetype = nullptr, IEntityClass* entityClass = nullptr);
	// Dumping Utilities

public:
	
	EntityUtils();
	ArkPlayer* ArkPlayerPtr();

	// ArchetypeLibrary entityArchetypeLibrary;
	ArkAbilityLibrary abilityLibrary;
	
};

extern EntityUtils *gEntUtils;
