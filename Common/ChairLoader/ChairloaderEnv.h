#pragma once
#include "IChairloader.h"
#include "IChairloaderEnv.h"

struct ChairloaderGlobalEnvironment : IChairloaderGlobalEnvironment {
	IChairloader* cl;
	IChairloaderGui* gui;
	IChairloaderConfigManager* conf;
	EntityUtils* entUtils;

};

// extern ChairloaderGlobalEnvironment* gCLEnv;