#pragma once
#include <memory>
#include <Prey/CrySystem/ISystem.h>

#include "ChairloaderConfigManager.h"
#include "ChairLoaderImGui.h"
#include "ChairLoader/IChairloader.h"
#include "Logging.h"
#include "ChairLoader/IChairloaderMod.h"

#define PREY_DLL_NAME "PreyDll.dll"

struct IChairloaderModuleManager;
struct SInputEvent;
struct IGameFramework;
class CSystem;

class ChairLoader : public virtual IChairloader {
public:
	class ModEntry {
	public:
		ModEntry(const std::string& mod_name, IChairloaderMod* mod_interface, HMODULE module_handle, int load_order)
			: modName(mod_name),
			  modInterface(mod_interface),
			  moduleHandle(module_handle),
			  loadOrder(load_order) {}

		int getLoadOrder() const { return loadOrder; }
		~ModEntry() {}
		bool operator<(const ModEntry& rhs) const noexcept { return this->getLoadOrder() < rhs.getLoadOrder(); }
		bool operator==(const ModEntry& rhs) const noexcept { return this->modName == rhs.modName; }
		ModEntry& operator=(const ModEntry& rhs) noexcept { this->modName = rhs.modName;
			this->modInterface = rhs.modInterface;
			this->moduleHandle = rhs.moduleHandle;
			this->loadOrder = rhs.loadOrder;
			return *this; }
		// unique modName
		std::string modName = "";
		// interface for calling guaranteed functions
		IChairloaderMod* modInterface = nullptr;
		// handle for the mod 
		HMODULE moduleHandle = nullptr;
		// configuration file
		// pugi::xml_document configFile;
	private:
		int loadOrder = -1;
	};
	
	//! Constructed just after loading PreyDll.dll, before any game code is run.
	ChairLoader();

	//! Called during CSystem::Init, before any engine modules.
	void InitSystem(CSystem* pSystem);

	//! Called after CGame::Init
	void InitGame(IGameFramework* pFramework);

	//! Called before CGame::Shutdown.
	void ShutdownGame();

	//! Called before CSystem::Shutdown.
	void ShutdownSystem();

	//! Destroyed after CSystem::Shutdown
	~ChairLoader();

	//! Before CGame::Update
	void PreUpdate(bool haveFocus, unsigned int updateFlags);

	//! After CGame::Update
	void PostUpdate(bool haveFocus, unsigned int updateFlags);

	//! Process a key press.
	//! @returns whether it was handled and shouldn't be passed over to the game.
	bool HandleKeyPress(const SInputEvent &event);

	//! Process smoke form exit TODO: move this to separate mod
	void SmokeFormExit();

	//! Register a mod 
	bool RegisterMod(std::string modName, IChairloaderMod* modInterface, HMODULE moduleBase);

	inline std::thread::id GetMainThreadId() { return m_MainThreadId; }
	inline std::thread::id GetRenderThreadId() { return m_ImGui->GetRenderThreadId(); }
	inline uintptr_t GetModuleBase() { return m_ModuleBase; }
	inline IGameFramework* GetFramework() { return m_pFramework; }

private:
	//! The mod list
	std::vector<ModEntry> modList;

	//! intermediate structure to keep track of mod load order before the mods are registered
	std::map<std::string, int> modLoadOrder;

	//! reads the config file to obtain the mod load order
	void ReadModList();

	//! sorts the modList by load order
	void sortLoadOrder();

	

	//! load all mod DLL's from the mod names obtained from the config Mod List
	void initializeMods();

	//! load all registered mod configs
	void loadAllConfigs();

	/* Modules */
	IChairloaderModuleManager* moduleManager = nullptr;

	std::map<std::string, std::shared_ptr<IChairloaderModule>> modules;

	void loadModuleManager();

	const std::string chairloaderModName = "Chairloader";
	uintptr_t m_ModuleBase = 0;
	IGameFramework* m_pFramework = nullptr;
	std::unique_ptr<ChairLoaderImGui> m_ImGui;
	FILE *m_pConsoleFile = nullptr;
	std::thread::id m_MainThreadId;

	int m_FreeCamKey = 0;
	bool m_DevMode;
	bool m_FreeCamEnabled = false;

	void CreateConsole();
	void InstallHooks();
	void UpdateFreeCam();
protected:
	pugi::xml_node chairloaderModulesConfigNode;
	pugi::xml_node getModuleConfigNode(std::string modName) override;
public:
	IChairloaderGlobalEnvironment* getChairloaderEnvironment() override;
};

extern ChairLoader *gCL;
extern SSystemGlobalEnvironment *gEnv;
