// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include "mem.h"
#include "preyFunctions.h"
#include "proc.h"
#include "preyDllObjects.h"
#include "preyFunctions.h"
#include "ChairloaderUtils.h"
#include "ChairloaderGUI.h"
#include "ChairLoader.h"
// #include "ArkEntityArchetypeLibrary.h

#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


// #include "pugixml.hpp"


// Boolean




// Gui variables
// float my_color[4];
// const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
// const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

std::mutex safeToEject;
// GUI DRAWING FUNCTIONS

//Chairloader vars

// static bool spawnEntity = false;
// static uint64_t entityToBeSpawned = 0;
static bool devMode = false;
static bool freeCam = false;
int GuiToggleKey, freeCamKey;
static ChairloaderGui* gui = nullptr;

DWORD WINAPI ChairloaderThread(HMODULE hModule) {
	// Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "Welcome to funland sonic\n";
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"PreyDll.dll");
    chairloader = new ChairloaderUtils(moduleBase);

    gui = new ChairloaderGui(chairloader);

    std::ifstream ifile;
    char buf[MAX_PATH];
    GetModuleFileNameA(hModule, buf, MAX_PATH);
    std::string workingDir = buf;
    workingDir.erase(workingDir.find("ChairLoader.dll"));
    
    printf("path = %s\n", buf);
    printf("newpath = %s\n", workingDir.c_str());
    // ifile.open("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Prey\\Binaries\\Danielle\\x64\\Release\\chairloaderconfig.xml");
    ifile.open(workingDir + "chairloaderconfig.xml");
    if (ifile.is_open()) {
        printf("file exists\n");
    }
    else {
        printf("file doesn't exist\n");
        // return(-1);
    }  
    ifile.close();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file((workingDir + "chairloaderconfig.xml").c_str());
    std::cout << doc.child("config").child("filepaths").child("PreyDirectory").attribute("path").value() << std::endl;
    for(auto itr = doc.child("config").child("keybinds").children().begin(); itr != doc.child("config").child("keybinds").children().end(); ++itr) {
        std::cout << itr->attribute("name").name() << "=" << itr->attribute("name").value() << std::endl;
        std::cout << itr->attribute("key").name() << "=" << itr->attribute("key").value() << std::endl;
    }
    std::cout << std::endl << doc.child("config").child("keybinds").find_child_by_attribute("action", "name", "GUIToggle").attribute("key").value() << std::endl;
    // Get Module base
    
    GuiToggleKey = std::stoi(doc.child("config").child("keybinds").find_child_by_attribute("action", "name", "GUIToggle").attribute("key").value());
    freeCamKey = std::stoi(doc.child("config").child("keybinds").find_child_by_attribute("action", "name", "FreeCam").attribute("key").value());
    // std::cout << std::hex << GuiToggleKey << std::endl;

    // bypass thread check for entity Spawning
    mem::Nop((BYTE*)(moduleBase + 0x020e2c5), 20);
    

    std::cout << "ChairLoader Initialized...\n";
    std::cout << "\nModule Base: 0x" << std::hex << moduleBase << std::dec << "\n\n";

    // Chairloader Variables or something
    bool bConsole = false;
    bool bGloo = 0;
    bool thirdPerson = false;
    
    bool lockFreeCam = false;
    bool oldFreeCam = false;
    // std::vector<CEntity*> spawnedEntities; 
    while(true) {
        // key input
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }
        //free cam
        if (freeCam != oldFreeCam) {
            // freeCam = !freeCam;
            oldFreeCam = freeCam;
            printf("Freecam state: %u\n", freeCam);
            if(freeCam) {
                devMode = true;
                chairloader->internalPreyFunctions->CSystemF->setDevMode(chairloader->preyEnvironmentPointers->pSystem, devMode);
                chairloader->preyEnvironmentPointers->pGame->m_pConsole->ExecuteString((char*)"FreeCamEnable", false, true);
            } else {
                devMode = true;
                chairloader->internalPreyFunctions->CSystemF->setDevMode(chairloader->preyEnvironmentPointers->pSystem, devMode);
                chairloader->preyEnvironmentPointers->pGame->m_pConsole->ExecuteString((char*)"FreeCamDisable", false, true);
            }
        }
        // spawn Entity
        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            std::vector<ArkAbilityData>* abilities = &chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get()->m_abilities;
            if (!abilities->empty()) {
                //ImGui::Text("Size: %d\n", abilities->size());
                printf("Size: %llu\n", abilities->size());
                int clip = 0;
                for (auto itr = abilities->begin(); itr != abilities->end() && clip < abilities->size() + 100000; ++itr, clip++) {
                    // printf("%llu\n", itr->m_id);
                    if (itr->m_id != 0) {
                        static ArkAbilityLibrary library;
                        auto entry = library.arkAbilityMap.find(itr->m_id);
                        if (entry != library.arkAbilityMap.end()) {
                            // ImGui::Text(entry->second.c_str());
                            printf("%s\n", entry->second.c_str());
                            printf("Seen: %d\n Acquired: %d\n", itr->m_bSeen, itr->m_bAcquired);
                        }
                        else {
                            // ImGui::Text(std::to_string(itr->m_id).c_str());
                            // printf("%llu\n", itr->m_id);
                        }

                    }
                }
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            ArkAbilities* abilitiesPtr;
            // std::unordered_map<const ArkClass*, std::unique_ptr<ArkReflectedLibrary>>* classes = &chairloader->CGamePtr->m_pArkGameDataManager.get()->m_libraries;
            // for(auto itr = classes->begin(); itr != classes->end(); ++itr) {
            //     printf("%s: %p\n", itr->first->m_name, itr->second.get());
            //     if (strcmp(itr->first->m_name, "ArkAbilities") == 0) {
            //         abilitiesPtr = (ArkAbilities*)itr->second.get();
            //     }
            // }
            
            
        }
        if(GetAsyncKeyState(VK_NUMPAD4) & 1) {
            //if the player pointer is null do nothing
            auto acquired = chairloader->internalPreyFunctions->ArkAbilityComponentF->GetAcquiredAbilities(chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get());
            printf("%llu\n", acquired.size());
            for (auto itr = acquired.begin(); itr != acquired.end(); ++itr) {
                printf("%llu\nacquired: %u\n", (*itr)->m_id, (*itr)->m_bAcquired);
            }
        }
		
	}
    // cleanup & eject
    if (f) {
        fclose(f);
    }
    FreeConsole();
    
    // DetourDetach(&(LPVOID&)fnIDXGISwapChainPresent, (PBYTE)Present);
    // DetourTransactionCommit(); 
    FreeLibraryAndExitThread(hModule, 0);
}



class chairloaderGuiExtModTestMenu {
public:
	bool bShow{true};
    std::string message;
    float col[4]{ 1,1,1,1 };
    const std::string modName = "testMod1";
    void draw() {
        if (bShow) {
            ImGui::Begin("testMenu");
            ImGui::Text("This is another mod");
            ImGui::ColorPicker4("Cool things", col);
            ImGui::End();
        }
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("Another Mod")) {
            ImGui::MenuItem("I can do something");
            if (bShow) {
                if (ImGui::Button("Get Rid of Me"))
                    bShow = false;
            } else {
                if (ImGui::Button("Bring Me Back"))
                    bShow = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    std::function<void(void)> invokeDraw = [=]() {
        this->draw();
    };
};

/*
DWORD WINAPI GUIThread(HMODULE hModule) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    while (gui == nullptr) {
        // wait for chairloader to not be fucking dead
    }
    // gui object initialized
    
    // begin gui hooking
    GetPresent();
    if (!g_PresentHooked) {
        retrieveValues();
    }
    // detour time
    detourDirectXPresent();
    while (!g_bInitialised) {
        Sleep(1000);
    }
    printValues();
    
    printf("--Detour successful--\n");
    chairloaderGuiExtModTestMenu testMenu;
    

    if(gui->addDrawFunction(testMenu.modName, testMenu.invokeDraw))
		gui->logItem("Test Mod Loaded", testMenu.modName);

    while(true) {
        if (GetAsyncKeyState(VK_END) & 1) {
            safeToEject.lock();
            break;
        }
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            g_ShowMenu = !g_ShowMenu;
        }
       
        if(GetForegroundWindow() != window) {
            g_ShowMenu = false;
            // wentOutOfFocus = true;
            // printf("went out of focus\n");
        }
        //TODO: work out this shit
        gui->Handlers();
       
    	// if(GetForegroundWindow() == window && wentOutOfFocus) {
     //        g_ShowMenu = true;
     //        wentOutOfFocus = false;
     //        printf("back in baby\n");
     //    }
    }
    g_ShowMenu = false;
    // g_PresentHooked = false;
    safeToEject.unlock();
    FreeLibraryAndExitThread(hModule, 0);
}
*/




BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        gCL = new ChairLoader();
        // CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ChairloaderThread, hModule, 0, nullptr));
        // CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)GUIThread, hModule, 0, nullptr));
        // CloseHandle(CreateThread(nullptr, 0 (LPTHREAD_START_ROUTINE), hModule, 0, nullptr));
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // killDetourDirectX();
        break;
    }
    return TRUE;
}

