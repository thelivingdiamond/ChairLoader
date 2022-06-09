#pragma once
#include <Prey/ArkEntityArchetypeLibrary.h>
#include <Prey/ArkEntityClassLibrary.h>
#include <Prey/ArkAbilityLibrary.h>
#include "pch.h"

#include "preyDllObjects.h"
#include "PreyFunctions.h"
class EntityUtils {
public:
	enum class EntityType {
		human = 0,
		mimic = 1,
		phantom = 2,
		operators = 3
	};

	// entities (WIP)
	CEntity* CreateEntity(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, PreyFunctions* functions);
	CEntity* CreateEntityBasic(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, PreyFunctions* functions);
	SEntitySpawnParams* CreateEntitySpawnParameters(char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, SEntitySpawnParams* params);

	// Dumping Utilities
	void DumpEntity(CEntity* entity, bool dumpProxies);
	void DumpGameObject(CGameObject* obj);

	class NpcSpawnHelper {
	public:
		NpcSpawnHelper();
		std::unordered_map<uint64_t, CArkNpcSpawner*> managedSpawners; // maybe?
		const char* SetEntityArchetype(uint64_t archetypeId, IEntity* npcSpawner);
		const char* SetEntityArchetype(const char* archetypeName, IEntity* npcSpawner);
		IEntityArchetype* GetEntityArchetype(CArkNpcSpawner* npcSpawner);
		// CArkNpcSpawner* CreateNewNpcSpawner(uint64_t archetypeId, char* name);
		const char* GetNextUniqueName();
		const char* GetLastUniqueName();
		IEntity* GetVictimSpawnerEntity(EntityType type);
		IEntity* SpawnNpc(CArkNpcSpawner* spawner, const char* name);
		// std::vector<IEntity*> SpawnNpc(CArkNpcSpawner* spawner, char* name, uint32_t spawnCount);
		IEntity* SpawnNpcFromArchetype(uint64_t archetypeId, const char* name, EntityType type);
		std::vector<IEntity*> SpawnNpcFromArchetype(uint64_t archetypeId, const char* name, EntityType type, Vec3_tpl<float>* pos, uint32_t spawnCount);
		IEntity* SpawnNpcFromArchetype(uint64_t archetypeId, EntityType type);
		std::vector < IEntity*> SpawnNpcFromArchetype(uint64_t archetypeId, EntityType type, uint32_t spawnCount);

	private:
		std::string lastUniqueName;
		std::string nextUniqueName;
		int generatedCount;
	};
public:
	EntityUtils();
	NpcSpawnHelper spawnerHelper;
	
	ArkPlayer* ArkPlayerPtr();

	// ArchetypeLibrary entityArchetypeLibrary;
	ArkAbilityLibrary abilityLibrary;
	
};

extern EntityUtils *gEntUtils;