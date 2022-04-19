#pragma once
#include "pch.h"

#include "preyDllObjects.h"
#include "preyFunctions.h"
namespace ChairloaderUtils {
	enum class EntityType {
		human = 0,
		mimic = 1,
		phantom = 2,
		operators = 3
	};
	// entities (WIP)
	CEntity* CreateEntity(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, preyFunctions* functions);
	CEntity* CreateEntityBasic(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, preyFunctions* functions);
	SEntitySpawnParams* CreateEntitySpawnParameters(char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, SEntitySpawnParams* params);

	// Dumping Utilities
	void dumpEntity(CEntity* entity, bool dumpProxies);
	void dumpGameObject(CGameObject* obj);

	class NpcUtils {
	public:
		NpcUtils();
		
	};

	class NpcSpawnerManager {
	public:
		NpcSpawnerManager(preyFunctions*,staticObjectPointers*);
		preyFunctions* privateFuncs;
		staticObjectPointers* staticObjects;
		std::unordered_map<uint64_t, CArkNpcSpawner*> managedSpawners; // maybe?
		char* setEntityArchetype(uint64_t archetypeId, IEntity* npcSpawner);
		char* setEntityArchetype(char* archetypeName, IEntity* npcSpawner);
		IEntityArchetype* getEntityArchetype(CArkNpcSpawner* npcSpawner);
		CArkNpcSpawner* createNewNpcSpawner(uint64_t archetypeId, char* name);
		char* getNextUniqueName();
		char* getLastUniqueName();
		IEntity* getVictimSpawnerEntity(EntityType type);
		IEntity* spawnNpc(CArkNpcSpawner* spawner, char* name);
		std::vector<IEntity*> spawnNpc(CArkNpcSpawner* spawner, char* name,uint32_t spawnCount);
		IEntity* spawnNpcFromArchetype(uint64_t archetypeId, char* name, EntityType type);
		std::vector<IEntity*> spawnNpcFromArchetype(uint64_t archetypeId, char* name, EntityType type,Vec3_tpl<float> * pos, uint32_t spawnCount);
		IEntity* spawnNpcFromArchetype(uint64_t archetypeId, EntityType type);
		std::vector < IEntity*> spawnNpcFromArchetype(uint64_t archetypeId, EntityType type, uint32_t spawnCount);
	private:
		std::string lastUniqueName;
		std::string nextUniqueName;
		int generatedCount;
	};
}
