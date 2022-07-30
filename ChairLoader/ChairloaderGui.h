#pragma once
#include "pch.h"
#include <Prey/CrySystem/System.h>
#include <Prey/ArkEntityArchetypeLibrary.h>
#include "OverlayLog.h"
#include "PlayerManager.h"
#include "ChairloaderConfigManager.h"
#include "GUIUtils.h"
#include "PerfOverlay.h"
#include "Profiler.h"
#include "DevConsoleDialog.h"
#include "ChairLoader/IChairloaderGui.h"
// #include <stack>

class ChairloaderGui : IChairloaderGui {
public:
    ChairloaderGui();

    bool IsDevConsoleVisible() { return control.showDevConsole; }
    void SetDevConsoleVisible(bool state) { control.showDevConsole = state; }

    void logItem(std::string msg, const std::string modName, logLevel level = logLevel::normal, bool displayToScreen = true) override;
    void logItem(logMessage message, bool displayToScreen = true) override;

    void draw(bool* bShow);
    void update();

    void dockingTest();
private:
    struct chairloaderGuiControl {
        bool showPlayerManager = false,
            showEntityList = false,
            showEntitySpawner = false,
            showLogHistory = false,
            showDemoWindow = false,
            showStyleManager = false,
            showProfilerDialog = false,
            showDevConsole = false,
			showConfigMenu = false,
            freeCam = false,
            devMode = false,
            hideAll = false;
    };
    const std::string modName = "ChairloaderGUI";
    //TODO: rethink the control variable
    
    bool g_PresentHooked;
    bool g_ShowMenu;
    bool g_bInitialised;


    //TODO: switch to module internal draw booleans
    chairloaderGuiControl control;
    ChairloaderGUILog log;
    ChairloaderGUIPlayerManager playerManager; 
    PerfOverlay perfOverlay;
    ProfilerDialog profilerDialog;
    DevConsoleDialog devConsoleDialog;

    //TODO: remove mutex bc single thread
    std::mutex drawHandleMutex;

};

extern ChairloaderGui *gui;
