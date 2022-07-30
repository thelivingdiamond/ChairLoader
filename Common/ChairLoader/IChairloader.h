#pragma once
#include <EntityUtils.h>

#include "IChairloaderEnv.h"
#include "IChairloaderConfigManager.h"
#include "IChairloaderGui.h"

class IChairloaderModule;

struct IChairloader {
	// TODO: figure out what goes here!!!'
	virtual IChairloaderGlobalEnvironment* getChairloaderEnvironment() = 0;
	virtual uintptr_t GetModuleBase() = 0;
	friend IChairloaderModule;
protected:
	virtual pugi::xml_node getModuleConfigNode(std::string modName) = 0;
};

