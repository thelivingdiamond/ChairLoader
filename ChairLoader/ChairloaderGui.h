#pragma once
#include "pch.h"

#include "ArkBasicTypes.h"
#include "ChairloaderUtils.h"
#include "preyDllObjects.h"
#include "ArkEntityArchetypeLibrary.h"
#include "ChairloaderGUILog.h"
#include "ChairloaderGUIEntityManager.h"
#include "ChairloaderGUIPlayerManager.h"
#include "ChairloaderGUIConsole.h"
#include "GUIUtils.h"
// #include <stack>
//TODO: INCLUDE LISTENER/HANDLER FUNCTIONS

class ChairloaderGui {

private:
    ChairloaderUtils* chairloaderGlobal;
    const std::string modName = "ChairloaderGUI";
    //TODO: rethink this one
    struct chairloaderGuiControl {
        bool showConsole = false,
            showPlayerManager = false,
            showEntityManager = false,
            showLogHistory = false,
            showDemoWindow = false,
            showStyleManager = false,
            freeCam = false,
            devMode = false;
    };
    bool g_PresentHooked;
    bool g_ShowMenu;
    BOOL g_bInitialised;
    // static ChairloaderGui* gui;

    chairloaderGuiControl control;
    ChairloaderGUILog log;
    ChairloaderGUIPlayerManager playerManager; 
    ChairloaderGUIEntityManager entityManager;
    ChairloaderGUIConsole console;
    // std::vector<std::string> modsWithDrawFuncs;
    std::vector<std::tuple<std::function<void()>, std::string>> drawFuncs;
    std::mutex drawHandleMutex;
public:
    ChairloaderGui(ChairloaderUtils* chairloader_global)
	    : chairloaderGlobal(chairloader_global),
        playerManager(),
        entityManager(chairloader_global),
        console() {
        // gui = this;
    }

    void logItem(std::string msg, const std::string modName, ChairloaderGUILog::logLevel level = ChairloaderGUILog::logLevel::normal, bool displayToScreen = true) {
        log.logItem(msg, modName, level, displayToScreen);
    }
    void logItem(ChairloaderGUILog::logMessage message, bool displayToScreen = true) {
        log.logItem(message, displayToScreen);
    }
    void draw(bool* bShow) {
        if (*bShow) {
            drawHandleMutex.lock();
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);
            if (ImGui::BeginMainMenuBar()) {
                // if(ImGui::IsItemHovered()) {
                //     ImGui::PopStyleVar();
                //     alphaVar = false;
                // }
                if (ImGui::BeginMenu("Chairloader")) {
                    ImGui::MenuItem("Show Console", NULL, &control.showConsole);
                    ImGui::MenuItem("Show Player Manager", NULL, &control.showPlayerManager);
                    ImGui::MenuItem("Show Entity Manager", NULL, &control.showEntityManager);
                    ImGui::Separator();
                    ImGui::MenuItem("Show Log History", nullptr, &control.showLogHistory);
                    ImGui::Separator();
                    ImGui::MenuItem("Show ImGui Demo", NULL, &control.showDemoWindow);
                    ImGui::MenuItem("Show Style Editor", NULL, &control.showStyleManager);
                    // ImGui::ShowStyleEditor();
                    if (ImGui::MenuItem("Test Log Message")) {
                        std::string testMsg = "I've come to make an announcement: Shadow the Hedgehog's a bitch-ass motherfucker. He pissed on my fucking wife. That's right. He took his hedgehog fuckin' quilly dick out and he pissed on my FUCKING wife, and he said his dick was THIS BIG";
                        // std::string testMsg = "I've come to make an announcement: Shadow the Hedgehog's a bitch-ass motherfucker. He pissed on my fucking wife. That's right. He took his hedgehog fuckin' quilly dick out and he pissed on my FUCKING wife, and he said his dick was THIS BIG, and I said that's disgusting. So I'm making a callout post on my Twitter.com. Shadow the Hedgehog, you got a small dick. It's the size of this walnut except WAY smaller. And guess what? Here's what my dong looks like. That's right, baby. Tall points, no quills, no pillows, look at that, it looks like two balls and a bong. He fucked my wife, so guess what, I'm gonna fuck the earth. That's right, this is what you get! My SUPER LASER PISS! Except I'm not gonna piss on the earth. I'm gonna go higher. I'm pissing on the MOOOON! How do you like that, OBAMA? I PISSED ON THE MOON, YOU IDIOT! You have twenty-three hours before the piss DROPLETS hit the fucking earth, now get out of my fucking sight before I piss on you too! ";
                        log.logItem(testMsg, modName);
                    }
                    if (ImGui::MenuItem("Test Error Message")) {
                        log.logItem("Welcome to funland sonic", modName, ChairloaderGUILog::logLevel::error);
                    }
                    ImGui::EndMenu();
                }
                playerManager.drawMenuBar(&control.showPlayerManager);
                entityManager.drawMenuBar(&control.showEntityManager, chairloaderGlobal);
                ImGui::EndMainMenuBar();
            }
            
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("Console")) {
                    ImGui::MenuItem("Free Cam", NULL, &control.freeCam);
                    ImGui::EndMenu();
                    // }
                }
                ImGui::EndMainMenuBar();
            }
            if (!drawFuncs.empty()) {
                for (auto itr = drawFuncs.begin(); itr != drawFuncs.end(); ++itr) {
                    std::get<0>(*itr)();
                }
            }
            if (control.showDemoWindow)
                ImGui::ShowDemoWindow(&control.showDemoWindow);
            if (control.showStyleManager)
                ImGui::ShowStyleEditor();
            if(control.showLogHistory)
                log.drawHistory(&control.showLogHistory);
            log.drawDisplay();
            if(control.showEntityManager)
                entityManager.draw(&control.showEntityManager);
            if(control.showPlayerManager)
                playerManager.draw(&control.showPlayerManager);
            if(control.showConsole)
                console.Draw(&control.showConsole);
            
            drawHandleMutex.unlock();
            
        }
    }
    void update() {
        if (!chairloaderGlobal->preyEnvironmentPointers->pGame->m_pFramework->IsInLevelLoad() || !chairloaderGlobal->preyEnvironmentPointers->pGame->m_pFramework->IsLoadingSaveGame()) {
            drawHandleMutex.lock();
            entityManager.update(chairloaderGlobal, &log);
            playerManager.update(chairloaderGlobal, &log);
            //TODO: run other GUI handlers in here 
            drawHandleMutex.unlock();
        }
    }
    void hookDirectX11Present() {

    }
    bool addDrawFunction(std::string modName, std::function<void()> drawFunction) {
        if (std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; }) == drawFuncs.end()) {
            drawFuncs.emplace_back(std::make_tuple(drawFunction, modName));
            return true;
        }
        return false;
    }
    bool isDrawFunctionLoaded(const std::string modName) {
        return std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; }) != drawFuncs.end();
    }
    bool unloadDrawFunction(const std::string modName) {
        auto pos = std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; });
	    if(pos != drawFuncs.end()) {
            drawFuncs.erase(pos);
            return true;
	    }
        return false;
    }

    // void hook() {
    //     // begin gui hooking
    //     GetPresent();
    //     if (!g_PresentHooked) {
    //         retrieveValues();
    //     }
    //     // detour time
    //     detourDirectXPresent();
    //     while (!g_bInitialised) {
    //         Sleep(100);
    //     }
    //     printValues();
    // }

private:
    // LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    //
    // HRESULT GetDeviceAndCtxFromSwapchain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
    //
    // HRESULT __fastcall Present(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags);
    //
    // void detourDirectXPresent();
    //
    // void retrieveValues();
    //
    // void printValues();
    //
    // void GetPresent();

};

