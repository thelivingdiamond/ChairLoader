#include "StdAfx.h"
#include "ChairloaderUtils.h"
SEntitySpawnParams* ChairloaderUtils::
CreateEntitySpawnParameters(char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, SEntitySpawnParams* params) {
	params->vScale.x = 1;
	params->vScale.y = 1;
	params->vScale.z = 1;
	params->vPosition.x = pos->x;
	params->vPosition.y = pos->y;
	params->vPosition.z = pos->z;
	params->qRotation.x = rot->x;
	params->qRotation.y = rot->y;
	params->qRotation.z = rot->z;
	params->qRotation.w = rot->w;
	params->sLayerName = (char*)"";
	params->guid = 0;
	params->prevGuid = 0;
	params->pClass = nullptr;
	params->pArchetype = nullptr;
	params->entityNode.ptr = nullptr;
	params->id = 0;
	params->prevId = 0;
	params->nFlags = 0;
	params->nFlagsExtended = 0;
	params->bIgnoreLock = 0;
	params->bStaticEntityId = 0;
	params->bCreatedThroughPool = 0;
	params->sceneMask = 0x1;
	params->shadowCasterType = '\0';
	params->pPropertiesTable = nullptr;
	params->pPropertiesInstanceTable = nullptr;
	params->sName = name;
	return params;
}

void ChairloaderUtils::dumpEntity(CEntity* entity, bool dumpProxies) {
	if (entity != nullptr) {
		std::cout << "entity: " << entity << std::endl;
		std::cout << "id:" << entity->m_nID << std::endl;
		std::cout << "guid:" << entity->m_guid << std::endl;
		std::cout << "flags:" << entity->m_flags << std::endl;
		std::cout << "flags ext:" << entity->m_flagsExtended << std::endl;
		std::cout << "class:" << entity->m_pClass << std::endl;
		std::cout << "archetype:" << entity->m_pArchetype << std::endl;
		std::cout << "SBindings:" << entity->m_pBinds << std::endl;
		std::cout << "m_aiObjectID:" << entity->m_aiObjectID << std::endl;
		std::cout << "IMaterial:" << entity->m_pMaterial << std::endl;
		if (dumpProxies) {
			std::cout << "all proxies:\n";
			for (uint32_t i = 0; i <= 0x11; i++) {
				IEntityProxy* proxy = ((IEntity*)entity)->GetProxy((EEntityProxy)i);
				std::cout << proxy << std::endl;
			}
			std::cout << "included proxies:\n";
			for (auto it = entity->m_proxy.begin(); it != entity->m_proxy.end(); ++it) {
				// if(it != NULL)
				std::cout << it->first << ": " << it->second.get() << std::endl;
			}
		}
		else {
			std::cout << "proxy:" << &entity->m_proxy << std::endl;
		}
		std::cout << "component:" << &entity->m_components << std::endl;
		std::cout << "IEntityLink:" << entity->m_pEntityLinks << std::endl;
		std::cout << "SGridLocation:" << entity->m_pGridLocation << std::endl;
		std::cout << "ProximityEntity:" << entity->m_pProximityEntity << std::endl;
		std::cout << "keepAliveCounter:" << entity->m_nKeepAliveCounter << std::endl;
		std::cout << "Name:" << entity->m_szName.m_str << std::endl;
		std::cout << "cloneLayerId:" << entity->m_cloneLayerId << std::endl;
		std::cout << "initial scene Mask:" << entity->m_guid << std::endl;
		std::cout << "displayName:" << entity->m_displayName.m_str << std::endl << std::endl;
	} else {
		std::cout << "Null Entity" << std::endl << std::endl;
	}
}
void ChairloaderUtils::dumpGameObject(CGameObject* obj) {
	if (obj != nullptr) {
		std::cout << "CGameObject:" << obj << std::endl;
		std::cout << "Action Delegate:" << obj->m_pActionDelegate << std::endl;
		std::cout << "View Delegate:" << obj->m_pViewDelegate << std::endl;
		std::cout << "Profile Manager:" << obj->m_pProfileManager << std::endl;
		std::cout << "profiles:" << &obj->m_profiles << std::endl;
		std::cout << "user data:" << obj->m_pUserData << std::endl;
		std::cout << "Mutex:" << obj->m_mutex.ptr << std::endl;
		std::cout << "extensions:" << obj->m_extensions.begin()._Ptr << std::endl;
		std::cout << "channel ID:" << obj->m_channelId << std::endl;
		std::cout << "enabled aspects:" << obj->m_enabledAspects << std::endl;
		std::cout << "delegatable aspects:" << obj->m_delegatableAspects << std::endl;
		std::cout << "Game Object Extension Cached Key:" << obj->m_pGameObjectExtensionCachedKey.get() << std::endl;
		std::cout << "SExtension:" << obj->m_pGameObjectExtensionCachedValue << std::endl;
		std::cout << "update timer:" << obj->m_updateTimer << std::endl;
		std::cout << "current dist:" << obj->m_currDistFromPlayerSq << std::endl;
		std::cout << "prediction handle:" << obj->m_predictionHandle << std::endl;
		std::cout << "enabled physics:" << obj->m_enabledPhysicsEvents << std::endl;
		std::cout << "force update:" << obj->m_forceUpdate << std::endl;
		std::cout << "prediction handle:" << obj->m_predictionHandle << std::endl;
		std::cout << "Pre Physics Update Rule:" << obj->m_prePhysicsUpdateRule << std::endl;
		std::cout << "Scheduling Profiles:" << obj->m_pSchedulingProfiles << std::endl;
		std::cout << "current scheduling profile:" << obj->m_pActionDelegate << std::endl;
		std::cout << "cached parent ID:" << obj->m_pActionDelegate << std::endl << std::endl;;
	}
}


char* ChairloaderUtils::NpcSpawnHelper::getNextUniqueName() {
	lastUniqueName = nextUniqueName;
	generatedCount++;
	nextUniqueName = "newEntity";
	nextUniqueName += std::to_string(generatedCount);
	return (char*)lastUniqueName.c_str();
}
char* ChairloaderUtils::NpcSpawnHelper::getLastUniqueName() {
	return (char*)lastUniqueName.c_str();
}

IEntity* ChairloaderUtils::NpcSpawnHelper::getVictimSpawnerEntity(EntityType type) {
	IEntity* spawnerEntity = nullptr;
	if (type == EntityType::human) {
		spawnerEntity = staticObjects->pEntitySystem->FindEntityByName((char*)"Igwe");
	}
	else if (type == EntityType::mimic) {
		spawnerEntity = staticObjects->pEntitySystem->FindEntityByName((char*)"ArkNpcSpawner_Mimic1");
	}
	else if (type == EntityType::phantom) {
		spawnerEntity = staticObjects->pEntitySystem->FindEntityByName((char*)"ArkNpcSpawner_BasePhantom1");
	}
	else if (type == EntityType::operators) {
		spawnerEntity = staticObjects->pEntitySystem->FindEntityByName((char*)"ArkNpcSpawner_MedicalOperator1");
	}
	return spawnerEntity;
}

ChairloaderUtils::NpcSpawnHelper::NpcSpawnHelper(preyFunctions* functions, gameEnvironmentPointers* pointers) {
	privateFuncs = functions;
	staticObjects = pointers;
	generatedCount = 0;
	lastUniqueName = "";
	nextUniqueName = "newEntity0";
}
char* ChairloaderUtils::NpcSpawnHelper::setEntityArchetype(uint64_t archetypeId, IEntity* spawnerEntity) {
	if(privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}

	ArkSafeScriptTable EntityScriptTable;
	EntityScriptTable.ptr = nullptr;
	ArkSafeScriptTable PropertiesTable;
	PropertiesTable.ptr = nullptr;

	privateFuncs->ArkSafeScriptTablef->getArkSafeScriptFromEntity(&EntityScriptTable, (IEntity*)spawnerEntity);
	if (EntityScriptTable.ptr != nullptr) {
		ScriptAnyValue TableValue;
		ScriptAnyValue ArchetypeValue;
		ScriptAnyValue NewArchetypeValue;
		TableValue.type = ScriptAnyType::ANY_TTABLE;
		TableValue.value.table = nullptr;
		ArchetypeValue.type = ScriptAnyType::ANY_TSTRING;
		ArchetypeValue.value.str = nullptr;
		NewArchetypeValue.type = ScriptAnyType::ANY_TSTRING;

		CEntityArchetype* archetype = staticObjects->pEntitySystem->GetEntityArchetype(archetypeId);
		NewArchetypeValue.value.str = archetype->m_name.m_str;

		if (EntityScriptTable.ptr->GetValueAny((char*)"Properties", &TableValue, false)) {
			if (TableValue.value.table != nullptr) {
				privateFuncs->ArkSafeScriptTablef->getArkSafeScriptFromScriptTable(&PropertiesTable, TableValue.value.table);
				if (PropertiesTable.ptr->GetValueAny((char*)"sNpcArchetype", &ArchetypeValue, false)) {
					PropertiesTable.ptr->SetValueAny((char*)"sNpcArchetype", &NewArchetypeValue, false);
					return ArchetypeValue.value.str;
				}
			}
		}
	}
	return nullptr;
}
char* ChairloaderUtils::NpcSpawnHelper::setEntityArchetype(char* archetypeName, IEntity* spawnerEntity) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}

	ArkSafeScriptTable EntityScriptTable;
	EntityScriptTable.ptr = nullptr;
	ArkSafeScriptTable PropertiesTable;
	PropertiesTable.ptr = nullptr;

	privateFuncs->ArkSafeScriptTablef->getArkSafeScriptFromEntity(&EntityScriptTable, (IEntity*)spawnerEntity);
	if (EntityScriptTable.ptr != nullptr) {
		ScriptAnyValue TableValue;
		ScriptAnyValue ArchetypeValue;
		ScriptAnyValue NewArchetypeValue;
		TableValue.type = ScriptAnyType::ANY_TTABLE;
		TableValue.value.table = nullptr;
		ArchetypeValue.type = ScriptAnyType::ANY_TSTRING;
		ArchetypeValue.value.str = nullptr;
		NewArchetypeValue.type = ScriptAnyType::ANY_TSTRING;
		NewArchetypeValue.value.str = archetypeName;

		if (EntityScriptTable.ptr->GetValueAny((char*)"Properties", &TableValue, false)) {
			if (TableValue.value.table != nullptr) {
				privateFuncs->ArkSafeScriptTablef->getArkSafeScriptFromScriptTable(&PropertiesTable, TableValue.value.table);
				if (PropertiesTable.ptr->GetValueAny((char*)"sNpcArchetype", &ArchetypeValue, false)) {
					PropertiesTable.ptr->SetValueAny((char*)"sNpcArchetype", &NewArchetypeValue, false);
					return ArchetypeValue.value.str;
				}
			}
		}
	}
	return nullptr;
}


IEntityArchetype* ChairloaderUtils::NpcSpawnHelper::getEntityArchetype(CArkNpcSpawner* npcSpawner) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}

	return privateFuncs->CArkNpcSpawnCystoidF->getEntityArchetype((CArkNpcSpawnCystoid*)npcSpawner);
}
// CArkNpcSpawner* ChairloaderUtils::NpcSpawnHelper::createNewNpcSpawner(uint64_t archetypeId, char* name) {
// 	if (privateFuncs == nullptr || staticObjects == nullptr) {
// 		std::cout << "Error, private functions or static objects uninitialized";
// 		return nullptr;
// 	}
//
// }
IEntity* ChairloaderUtils::NpcSpawnHelper::spawnNpc(CArkNpcSpawner* spawner, char* name) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}
	if (spawner != nullptr) {
		char* oldName = spawner->m_Entity->m_szName.m_str;
		spawner->m_Entity->m_szName.m_str = name;
		uint32_t oldId = spawner->m_lastSpawnedEntityId;
		privateFuncs->CArkNpcSpawnerF->requestSpawn(spawner);
		while(spawner->m_lastSpawnedEntityId == oldId) {
			Sleep(1);
		}
		spawner->m_Entity->m_szName.m_str = oldName;
		// Sleep(50);
		IEntity* newEntity = newEntity = staticObjects->pEntitySystem->GetEntity(spawner->m_lastSpawnedEntityId);
		// Sleep(5);
		return newEntity;
	}
	return nullptr;
}
// std::vector<IEntity*> ChairloaderUtils::NpcSpawnHelper::spawnNpc(CArkNpcSpawner* spawner, char* name, uint32_t spawnCount) {
// 	if (privateFuncs == nullptr || staticObjects == nullptr) {
// 		std::cout << "Error, private functions or static objects uninitialized";
// 		return {};
// 	}
// 	std::vector<IEntity*> entities;
// 	if (spawner != nullptr) {
// 		char* oldName = spawner->m_Entity->m_szName.m_str;
// 		std::string strname = name;
// 		for(unsigned int i = 0; i<=spawnCount; i++) {
// 			strname += std::to_string(i);
// 			spawner->m_Entity->m_szName.m_str = (char*)strname.c_str();
// 			uint32_t oldId = spawner->m_lastSpawnedEntityId;
//
// 			privateFuncs->CArkNpcSpawnerF->requestSpawn(spawner);
//
// 			while(spawner->m_lastSpawnedEntityId == oldId) {
// 			// wait for shit to happen
// 			}
// 			IEntity* newEntity = newEntity = staticObjects->pEntitySystem->GetEntity(spawner->m_lastSpawnedEntityId);
// 			entities.emplace_back(newEntity);
// 		}
// 		spawner->m_Entity->m_szName.m_str = oldName;
// 		return entities;
// 	}
// 	return {};
// }
IEntity* ChairloaderUtils::NpcSpawnHelper::spawnNpcFromArchetype(uint64_t archetypeId, char* name, EntityType type) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}
	IEntity* spawnerEntity = getVictimSpawnerEntity(type);

	if (spawnerEntity != nullptr) {
		char* oldArchetypeName = setEntityArchetype(archetypeId, spawnerEntity);
		if (oldArchetypeName != nullptr) {
			CArkNpcSpawner* spawner = privateFuncs->CEntity->getArkNpcSpawner((CEntity*)spawnerEntity);
			IEntity* newEntity = spawnNpc(spawner, name);
			setEntityArchetype(oldArchetypeName, spawnerEntity);
			return newEntity;
		}
	}
	return nullptr;
}

std::vector<IEntity*> ChairloaderUtils::NpcSpawnHelper::spawnNpcFromArchetype(uint64_t archetypeId, char* name, EntityType type, Vec3_tpl<float> * pos, uint32_t spawnCount) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return {};
	}
	IEntity* spawnerEntity = getVictimSpawnerEntity(type);
	std::vector<IEntity*> newEntities;
	if (spawnerEntity != nullptr) {
		char* oldArchetypeName = setEntityArchetype(archetypeId, spawnerEntity);
		CArkNpcSpawner* spawner = privateFuncs->CEntity->getArkNpcSpawner((CEntity*)spawnerEntity);
		Vec3_tpl<float> oldpos = spawner->m_Entity->m_vPos;
		spawner->m_Entity->m_vPos.x = pos->x;
		spawner->m_Entity->m_vPos.y = pos->y;
		spawner->m_Entity->m_vPos.z = pos->z;
		for (auto i = 0; i <= spawnCount; i++) {
			IEntity* newEntity = spawnNpc(spawner, name);
			newEntity->SetPos(pos, 0, false, false);
			newEntities.emplace_back(newEntity);
		}
		spawner->m_Entity->m_vPos = oldpos;
		setEntityArchetype(oldArchetypeName, spawnerEntity);
		return newEntities;
	}
	return {};
}

IEntity* ChairloaderUtils::NpcSpawnHelper::spawnNpcFromArchetype(uint64_t archetypeId, EntityType type) {
	if (privateFuncs == nullptr || staticObjects == nullptr) {
		std::cout << "Error, private functions or static objects uninitialized";
		return nullptr;
	}
	IEntity* spawnerEntity = getVictimSpawnerEntity(type);

	if (spawnerEntity != nullptr) {
		char* oldArchetypeName = setEntityArchetype(archetypeId, spawnerEntity);
		CArkNpcSpawner* spawner = privateFuncs->CEntity->getArkNpcSpawner((CEntity*)spawnerEntity);
		IEntity* newEntity = spawnNpc(spawner, getNextUniqueName());
		setEntityArchetype(oldArchetypeName, spawnerEntity);
		return newEntity;
	}
	return nullptr;
}

std::vector<IEntity*> ChairloaderUtils::NpcSpawnHelper::spawnNpcFromArchetype(uint64_t archetypeId, EntityType type,
	uint32_t spawnCount) {
	return std::vector<IEntity*>{};
}

ChairloaderUtils::ChairloaderUtils(uintptr_t moduleBase) {
	internalPreyFunctions = new preyFunctions(moduleBase);
	preyEnvironmentPointers = (gameEnvironmentPointers*)(moduleBase + 0x22418c0);
	// CEntitySystemPtr = preyEnvironmentPointers->pEntitySystem;
	ArkPlayerPtr = internalPreyFunctions->ArkPlayerF->getInstance();
	// CGamePtr = preyEnvironmentPointers->pGame;
	spawnerHelper = new NpcSpawnHelper(internalPreyFunctions, preyEnvironmentPointers);

}


// CEntity* ChairloaderUtils::CreateEntity(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, preyFunctions* functions) {
// 	SEntitySpawnParams params;
// 	if (CreateEntitySpawnParameters(name, pos, rot, &params) == nullptr)
// 		return nullptr;
// 	CEntityArchetype* archetype = system->GetEntityArchetype(archetypeId);
// 	if (archetype == nullptr)
// 		return nullptr;
// 	CEntityClass* entityClass = (CEntityClass*)archetype->GetClass();
// 	if (entityClass == nullptr)
// 		return nullptr;
// 	params.pArchetype = archetype;
// 	params.pClass = entityClass;
// 	// XmlNodeRef* xmlnode = archetype->GetObjectVars();
// 	params.entityNode = archetype->m_ObjectVars;
// 	// std::cout << params->id << std::endl;
// 	// params.id = 69420;
// 	params.id = functions->CEntitySystemF->generateEntityId(system, true);
// 	CEntity newEntity;
// 	return (CEntity*)functions->CEntity->CEntityConstructor(&newEntity, &params);
// 	return nullptr;
// }
// CEntity* ChairloaderUtils::CreateEntityBasic(CEntitySystem* system, char* name, Vec3_tpl<float>* pos, Quat_tpl<float>* rot, uint64_t archetypeId, preyFunctions* functions) {
// 	SEntitySpawnParams params;
// 	if (CreateEntitySpawnParameters(name, pos, rot, &params) == nullptr)
// 		return nullptr;
// 	CEntityArchetype* archetype = system->GetEntityArchetype(archetypeId);
// 	if (archetype == nullptr)
// 		return nullptr;
// 	CEntityClass* entityClass = (CEntityClass*)archetype->GetClass();
// 	if (entityClass == nullptr)
// 		return nullptr;
// 	std::cout << archetype << std::endl;
// 	XmlNodeRef node = archetype->m_ObjectVars;
// 	// if (node == nullptr)
// 	// 	return nullptr;
// 	params.pArchetype = archetype;
// 	params.pClass = entityClass;
// 	// XmlNodeRef* xmlnode = archetype->GetObjectVars();
// 	params.entityNode = node;
// 	// std::cout << params->id << std::endl;
// 	// params.id = 69420;
// 	params.pPropertiesTable = archetype->m_pProperties.ptr;
// 	params.pPropertiesInstanceTable = archetype->m_pProperties.ptr;
// 	params.id = functions->CEntitySystemF->generateEntityId(system, true);
// 	CEntity* newEntity = new CEntity;
// 	return (CEntity*)functions->CEntity->CEntityConstructor(newEntity, &params);
// 	// ((IEntity*)newEntity)->Init((IEntity*)newEntity, &params);
// 	// ((IEntity*)newEntity)->SetProxy(EEntityProxy::ENTITY_PROXY_USER, 0x88);
// 	return newEntity;
// }


