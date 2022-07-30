#pragma once

struct IChairloader;
class IChairloaderModule;

struct IChairloaderModuleManager {
	virtual ~IChairloaderModuleManager() = default;
	virtual void init(ISystem* pSystem, IChairloader* chairloader) = 0;
	virtual std::map<std::string, std::shared_ptr<IChairloaderModule>>  getLoadedModules() = 0;
};
