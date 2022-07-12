#include "pch.h"
#include "ChairloaderGUI.h"

ChairloaderGui *gui = nullptr;

ChairloaderGui::ChairloaderGui() :
    playerManager(),
    entityManager() {
    // gui = this;
    GUILog = &log;
}


void ChairloaderGui::logItem(std::string msg, const std::string modName, ChairloaderGUILog::logLevel level, bool displayToScreen) {
    log.logItem(msg, modName, level, displayToScreen);
}
void ChairloaderGui::logItem(ChairloaderGUILog::logMessage message, bool displayToScreen) {
    log.logItem(message, displayToScreen);
}
void ChairloaderGui::draw(bool* bShow) {
    if (*bShow) {
        drawHandleMutex.lock();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);
        if (ImGui::BeginMainMenuBar()) {
            // if(ImGui::IsItemHovered()) {
            //     ImGui::PopStyleVar();
            //     alphaVar = false;
            // }
            if (ImGui::BeginMenu("Chairloader")) {
                ImGui::MenuItem("Hide All", NULL, &control.hideAll);
                ImGui::MenuItem("Show Console", NULL, &control.showDevConsole);
                ImGui::MenuItem("Show Player Manager", NULL, &control.showPlayerManager);
                ImGui::MenuItem("Show Entity List", NULL, &control.showEntityList);
                ImGui::MenuItem("Show Entity Spawner", NULL, &control.showEntitySpawner);
                ImGui::Separator();
                ImGui::MenuItem("Show Log History", nullptr, &control.showLogHistory);
                ImGui::Separator();

                if (ImGui::BeginMenu("Performance")) {
                    ImGui::MenuItem("Profiler", nullptr, &control.showProfilerDialog);
                    ImGui::Separator();
                    perfOverlay.ShowMenu();
                    ImGui::EndMenu();
                }

                ImGui::MenuItem("Show ImGui Demo", NULL, &control.showDemoWindow);
                ImGui::MenuItem("Show Style Editor", NULL, &control.showStyleManager);
                // ImGui::ShowStyleEditor();
                if (ImGui::MenuItem("Test Log Message")) {
                    // std::string testMsg = "I've come to make an announcement: Shadow the Hedgehog's a bitch-ass motherfucker. He pissed on my fucking wife. That's right. He took his hedgehog fuckin' quilly dick out and he pissed on my FUCKING wife, and he said his dick was THIS BIG";
                    std::string testMsg = "I've come to make an announcement: Shadow the Hedgehog's a bitch-ass motherfucker. He pissed on my fucking wife. That's right. He took his hedgehog fuckin' quilly dick out and he pissed on my FUCKING wife, and he said his dick was THIS BIG, and I said that's disgusting. So I'm making a callout post on my Twitter.com. Shadow the Hedgehog, you got a small dick. It's the size of this walnut except WAY smaller. And guess what? Here's what my dong looks like. That's right, baby. Tall points, no quills, no pillows, look at that, it looks like two balls and a bong. He fucked my wife, so guess what, I'm gonna fuck the earth. That's right, this is what you get! My SUPER LASER PISS! Except I'm not gonna piss on the earth. I'm gonna go higher. I'm pissing on the MOOOON! How do you like that, OBAMA? I PISSED ON THE MOON, YOU IDIOT! You have twenty-three hours before the piss DROPLETS hit the fucking earth, now get out of my fucking sight before I piss on you too! ";
                    log.logItem(testMsg, modName);
                }
                if (ImGui::MenuItem("Test Error Message")) {
                    log.logItem("Welcome to funland sonic", modName, ChairloaderGUILog::logLevel::error);
                }
                ImGui::EndMenu();
            }
            playerManager.drawMenuBar(&control.showPlayerManager);
            entityManager.drawMenuBar(&control.showEntityList, &control.showEntitySpawner);
            ImGui::EndMainMenuBar();
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Console")) {
                if (control.freeCam) {
                    ImGui::Text("Free Cam: enabled");
                }
                else {
                    ImGui::Text("Free Cam: disabled");
                }
                if (ImGui::Button("Toggle Free Cam")) {
                    control.freeCam = !control.freeCam;
                    if (control.freeCam) {
                        control.devMode = true;
                        ((CSystem*)gEnv->pSystem)->SetDevMode(control.devMode);
                        gEnv->pConsole->ExecuteString("FreeCamEnable", false, true);
                    }
                    else {
                        ((CSystem*)gEnv->pSystem)->SetDevMode(control.devMode);
                        gEnv->pConsole->ExecuteString("FreeCamDisable", false, true);
                    }
                }

                ImGui::EndMenu();
                // }
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowBgAlpha(0.0);
        ImGui::SetNextWindowSize(ImGui::GetWindowViewport()->Size);
        ImGui::Begin("Main Dockspace Window", 0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
        ImGui::DockSpace(ImGui::GetID("Main Dockspace"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoDockingInCentralNode);
        if (!drawFuncs.empty()) {
            for (auto itr = drawFuncs.begin(); itr != drawFuncs.end(); ++itr) {
                std::get<0>(*itr)();
            }
        }
        ImGui::End();
        if (!control.hideAll) {
            if (control.showDemoWindow)
                ImGui::ShowDemoWindow(&control.showDemoWindow);
            if (control.showStyleManager)
                ImGui::ShowStyleEditor();
            if (control.showLogHistory)
                log.drawHistory(&control.showLogHistory);
            log.drawDisplay();
            if (control.showEntitySpawner)
                entityManager.drawEntitySpawner(&control.showEntitySpawner);
            if (control.showEntityList)
                entityManager.drawEntityList(&control.showEntityList);
            if (control.showPlayerManager)
                playerManager.draw(&control.showPlayerManager);

            if (control.showProfilerDialog) {
                profilerDialog.Show(&control.showProfilerDialog);
            }

            if (control.showDevConsole) {
                devConsoleDialog.Show(&control.showDevConsole);
            }
        }
        log.drawDisplay();
        drawHandleMutex.unlock();

    }
    ImGui::PopStyleVar();
}

void ChairloaderGui::update() {
    auto pAction = reinterpret_cast<CCryAction*>(gCL->GetFramework());
    if (!pAction->IsInLevelLoad() || !pAction->IsLoadingSaveGame()) {
        drawHandleMutex.lock();
        entityManager.update(&log);
        playerManager.update(&log);
        drawHandleMutex.unlock();
    }
    perfOverlay.Update();
}
bool ChairloaderGui::addDrawFunction(std::string modName, std::function<void()> drawFunction) {
    if (std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; }) == drawFuncs.end()) {
        drawFuncs.emplace_back(std::make_tuple(drawFunction, modName));
        return true;
    }
    return false;
}
bool ChairloaderGui::isDrawFunctionLoaded(const std::string modName) {
    return std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; }) != drawFuncs.end();
}
bool ChairloaderGui::unloadDrawFunction(const std::string modName) {
    auto pos = std::find_if(drawFuncs.begin(), drawFuncs.end(), [modName](std::tuple < std::function<void()>, std::string>& e) {return std::get<1>(e) == modName; });
    if (pos != drawFuncs.end()) {
        drawFuncs.erase(pos);
        return true;
    }
    return false;
}

void ChairloaderGui::dockingTest() {
    if (ImGui::Begin("Docking Test 1")) {
        ImGui::DockSpace(ImGui::GetID("test 1"));
        ImGui::End();
    }
}
