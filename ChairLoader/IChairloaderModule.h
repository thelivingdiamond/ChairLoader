#pragma once
#include "ChairLoader/IChairloader.h"
#include "Chairloader/IChairloaderMod.h"

/**
 * \brief Internal chairloader module interface. Has access to protected chairloader functions
 */
class IChairloaderModule : public virtual IChairloaderMod {
public:
	virtual ~IChairloaderModule() = default;
	virtual std::string GetModuleName() = 0;
	virtual void InitSystem() = 0;
	virtual void InitGame(IGameFramework* pFramework) = 0;
private:
	void InitSystem(CSystem* pSystem, uintptr_t moduleBase) override {};
	void InitGame(IGameFramework* pFramework, IChairloader* chairloader) override {};
	std::string GetModName() override { return ""; };
};

