#include "pch.h"
#include "ModuleManager.h"
#include <Prey/CryCore/Platform/platform_impl.inl>
ModuleManager::ModuleManager(){}

void ModuleManager::init(ISystem* pSystem, IChairloader* chairloaderIn) {
	ModuleInitISystem(pSystem, "ChairloaderModules");
	chairloader = chairloaderIn;
	gCLEnv = (ChairloaderGlobalEnvironment*)chairloader->getChairloaderEnvironment();
	chairloader = chairloaderIn;
	gCLEnv = (ChairloaderGlobalEnvironment*)chairloader->getChairloaderEnvironment();

	/* EntityManager */
	entityManager = std::make_shared<EntityManager>(gCLEnv);
	if(isModuleEnabled(entityManager->GetModuleName()))
		loadedModules.insert(std::pair(entityManager->GetModuleName(), entityManager));

	PreyFunctionSystem::Init(chairloader->GetModuleBase());
}

bool ModuleManager::isModuleEnabled(std::string moduleName) {
	auto modulesVariant = gCLEnv->conf->getConfigValue("Chairloader", "Modules");
	auto ModulesNode = boost::get<pugi::xml_node>(modulesVariant);
	if (ModulesNode) {
		auto moduleNode = ModulesNode.child(moduleName.c_str());;
		if (moduleNode) {
			auto result = gCLEnv->conf->getNodeConfigValue(moduleNode, "Enable");
			if (result.type() == typeid(bool)) {
				return boost::get<bool>(result);
			}
		}
	}
	return false;
}


EXTERN_C __declspec(dllexport) IChairloaderModuleManager* ClModuleManager_Instantiate() {
	return new ModuleManager();
}