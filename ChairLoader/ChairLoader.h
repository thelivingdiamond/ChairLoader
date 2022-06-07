#include "preyFunctions.h"
<<<<<<< Updated upstream

class ChairLoader {
public:
=======
#include "ChairloaderGUI.h"

class ChairLoader {
public:
	ChairloaderGui* chairloaderGUI;
>>>>>>> Stashed changes
	// After CGame::Init
	ChairLoader();

	// TODO: Call this somewhere
	~ChairLoader();

	// Before CGame::Update
	void PreUpdate(bool haveFocus, unsigned int updateFlags);

	// After CGame::Update
	void PostUpdate(bool haveFocus, unsigned int updateFlags);

private:
	FILE *m_pConsoleFile = nullptr;
	preyFunctions::CGamePrivate::_Update m_CGameUpdate = nullptr;
	int m_GuiToggleKey = 0;
	int m_FreeCamKey = 0;
	bool m_DevMode;
	bool m_FreeCamEnabled = false;

	void CreateConsole();
	void HookGameUpdate(uintptr_t moduleBase);
<<<<<<< Updated upstream
=======
	void HookDirectX();
>>>>>>> Stashed changes
	void LoadConfigFile();
	void UpdateFreeCam();

	static int GameUpdate(CGame *_this, bool haveFocus, unsigned int updateFlags);
};
<<<<<<< Updated upstream

=======
>>>>>>> Stashed changes
extern ChairLoader *gCL;
