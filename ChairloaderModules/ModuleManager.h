#pragma once

#include <Prey/CryCore/Platform/platform_impl.inl>

#include "EntityManager.h"
#include "Chairloader/IChairloaderModuleManager.h"
#include "ChairLoader/IChairloaderEnv.h"
class ModuleManager: public IChairloaderModuleManager 
{
public:
	IChairloader* chairloader = nullptr;
	ChairloaderGlobalEnvironment* gCLEnv = nullptr;
	
	ModuleManager();

	std::map<std::string, std::shared_ptr<IChairloaderModule>>  getLoadedModules() override { return loadedModules; }

	void init(ISystem* pSystem, IChairloader* chairloader) override;

	~ModuleManager() override = default;
private:
	std::map<std::string, std::shared_ptr<IChairloaderModule>> loadedModules;
	std::shared_ptr<EntityManager> entityManager;
	bool isModuleEnabled(std::string moduleName);
};
