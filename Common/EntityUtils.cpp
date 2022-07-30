#include "pch.h"
#include <Prey/CryAction/GameObject.h>
#include <Prey/GameDll/ark/player/ArkPlayer.h>
#include <Prey/GameDll/ark/npc/ArkNpcSpawnManager.h>
#include "EntityUtils.h"

EntityUtils *gEntUtils = nullptr;


IEntity* EntityUtils::spawnNpcFromArchetype(const char* name, Vec3& pos, Quat& rot, uint64 archetypeId, unsigned spawnCount) {
	IEntity* latestEntity = nullptr;
	static ArkNpcSpawnedState_Alert alert;
	static boost::variant<ArkNpcSpawnedState_Alert, ArkNpcSpawnedState_Broken, ArkNpcSpawnedState_Dead, ArkNpcSpawnedState_Dormant> state = alert;
	for (int i = 1; i <= spawnCount; i++) {
		latestEntity = ArkNpcSpawnManager::CreateNpc(*gEnv->pEntitySystem->GetEntityArchetype(archetypeId), pos, rot, 0, state);
	}
	return latestEntity;
}

IEntity* EntityUtils::spawnEntityFromArchetype(const char* name, Vec3 pos, Quat rot, uint64 archetypeId, unsigned spawnCount) {
	IEntity* latestEntity = nullptr;
	for (int i = 1; i <= spawnCount; i++) {
		SEntitySpawnParams params;
		CreateEntitySpawnParameters(name, pos, rot, &params);
		latestEntity = gEnv->pEntitySystem->SpawnEntityFromArchetype(gEnv->pEntitySystem->GetEntityArchetype(archetypeId), params);
	}
	return latestEntity;
}

SEntitySpawnParams* EntityUtils::
CreateEntitySpawnParameters(const char* name, Vec3 pos, Quat rot, SEntitySpawnParams* params,  IEntityArchetype* entityArchetype, IEntityClass* entityClass) {
	params->vScale.x = 1;
	params->vScale.y = 1;
	params->vScale.z = 1;
	params->vPosition.x = pos.x;
	params->vPosition.y = pos.y;
	params->vPosition.z = pos.z;
	params->qRotation.v.x = rot.v.x;
	params->qRotation.v.y = rot.v.y;
	params->qRotation.v.z = rot.v.y;
	params->qRotation.w = rot.w;
	params->sLayerName = "";
	params->pClass = entityClass;
	if(entityArchetype != nullptr) {
		params->entityNode = entityArchetype->GetObjectVars();
	} else {
		params->entityNode = nullptr;
	}
	params->pArchetype = nullptr;
	params->guid = 0;
	params->prevGuid = 0;
	params->prevId = 0;
	params->bCreatedThroughPool = 0;
	params->bIgnoreLock = 0;
	params->bStaticEntityId = 0;
	params->nFlags = 0;
	params->nFlagsExtended = 0;
	params->sName = name;
	params->shadowCasterType = '\0';
	params->pUserData = (void*)0x0;
	params->sceneMask = '\0';
	return params;
}

void EntityUtils::DumpEntity(IEntity* entity, bool dumpProxies) {
	if (entity != nullptr) {
		std::cout << "entity: " << entity << std::endl;
		std::cout << "id:" << entity->GetId() << std::endl;
		std::cout << "guid:" << entity->GetGuid() << std::endl;
		std::cout << "flags:" << entity->GetFlags() << std::endl;
		std::cout << "flags ext:" << entity->GetFlagsExtended() << std::endl;
		std::cout << "class:" << entity->GetClass() << std::endl;
		std::cout << "archetype:" << entity->GetArchetype() << std::endl;
		//std::cout << "SBindings:" << entity->m_pBinds << std::endl;
		std::cout << "m_aiObjectID:" << entity->GetAIObjectID() << std::endl;
		std::cout << "IMaterial:" << entity->GetMaterial() << std::endl;
		if (dumpProxies) {
			std::cout << "all proxies:\n";
			for (uint32_t i = 0; i <= 0x11; i++) {
				IEntityProxy* proxy = entity->GetProxy((EEntityProxy)i);
				std::cout << proxy << std::endl;
			}
			//std::cout << "included proxies:\n";
			//for (auto it = entity->m_proxy.begin(); it != entity->m_proxy.end(); ++it) {
			//	// if(it != NULL)
			//	std::cout << it->first << ": " << it->second.get() << std::endl;
			//}
		}
		else {
			//std::cout << "proxy:" << &entity->m_proxy << std::endl;
		}
		//std::cout << "component:" << &entity->m_components << std::endl;
		//std::cout << "IEntityLink:" << entity->m_pEntityLinks << std::endl;
		//std::cout << "SGridLocation:" << entity->m_pGridLocation << std::endl;
		//std::cout << "ProximityEntity:" << entity->m_pProximityEntity << std::endl;
		//std::cout << "keepAliveCounter:" << entity->m_nKeepAliveCounter << std::endl;
		std::cout << "Name:" << entity->GetName() << std::endl;
		//std::cout << "cloneLayerId:" << entity->m_cloneLayerId << std::endl;
		//std::cout << "initial scene Mask:" << entity->m_guid << std::endl;
		//std::cout << "displayName:" << entity->m_displayName.c_str() << std::endl << std::endl;
	} else {
		std::cout << "Null Entity" << std::endl << std::endl;
	}
}
void EntityUtils::DumpGameObject(CGameObject* obj) {
	if (obj != nullptr) {
		std::cout << "CGameObject:" << obj << std::endl;
		std::cout << "Action Delegate:" << obj->m_pActionDelegate << std::endl;
		std::cout << "View Delegate:" << obj->m_pViewDelegate << std::endl;
		std::cout << "Profile Manager:" << obj->m_pProfileManager << std::endl;
		std::cout << "profiles:" << &obj->m_profiles << std::endl;
		std::cout << "user data:" << obj->m_pUserData << std::endl;
		//std::cout << "Mutex:" << obj->m_mutex.ptr << std::endl;
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



EntityUtils::EntityUtils() {
	
}

ArkPlayer* EntityUtils::ArkPlayerPtr() {
	return ArkPlayer::GetInstancePtr();
}

EntityUtils::~EntityUtils() {}



