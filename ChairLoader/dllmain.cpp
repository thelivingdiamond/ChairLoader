// dllmain.cpp : Defines the entry point for the DLL application.
#include "StdAfx.h"
#include "mem.h"
#include "preyFunctions.h"
#include "proc.h"
#include "preyDllObjects.h"
#include "preyFunctions.h"
#include "ChairloaderUtils.h"
#include "ChairloaderGui.h"
#include <Windows.h>
// #include "ArkEntityArchetypeLibrary.h
namespace ip = boost::interprocess;

#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } 


// #include "pugixml.hpp"
// D3X HOOK DEFINITIONS
typedef HRESULT(__fastcall* IDXGISwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall* ID3D11DrawIndexed)(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
// Definition of WndProc Hook. Its here to avoid dragging dependencies on <windows.h> types.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main D3D11 Objects
ID3D11DeviceContext* pContext = NULL;
ID3D11Device* pDevice = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
static IDXGISwapChain* pSwapChain = NULL;
static WNDPROC OriginalWndProcHandler = nullptr;
HWND window = nullptr;
IDXGISwapChainPresent fnIDXGISwapChainPresent;

// Boolean
BOOL g_bInitialised = false;
bool g_ShowMenu = false;
bool g_PresentHooked = false;

// Gui variables
float my_color[4];
const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

std::mutex safeToEject;
// GUI DRAWING FUNCTIONS

//Chairloader vars

// static bool spawnEntity = false;
// static uint64_t entityToBeSpawned = 0;
static bool devMode = false;
static bool freeCam = false;
int GuiToggleKey, freeCamKey;
static ChairloaderUtils* chairloader = nullptr;
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

    ip::managed_shared_memory segment(ip::create_only, "ChairloaderBase", 65536);
    std::pair<ChairloaderUtils*, ChairloaderGui*>* instance = segment.construct<std::pair<ChairloaderUtils*, ChairloaderGui*>>("EnvInstance")(chairloader, gui);



    //TODO: figure out config loading for the chairloader DLL
    std::ifstream ifile;
    char buf[MAX_PATH];
    GetModuleFileNameA(hModule, buf, MAX_PATH);
    std::string workingDir = buf;
    // workingDir.erase(workingDir.find("ChairLoader.dll"));
    
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
            std::vector<ArkAbilityData>* abilities = &chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get()->m_abilities;
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
            auto acquired = chairloader->internalPreyFunctions->ArkAbilityComponentF->GetAcquiredAbilities(chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get());
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


LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(window, &mPos);
	ImGui::GetIO().MousePos.x = mPos.x;
	ImGui::GetIO().MousePos.y = mPos.y;

	// if (uMsg == WM_KEYUP)
	// {
	// 	if (wParam == VK_INSERT)
	// 	{
	// 		g_ShowMenu = !g_ShowMenu;
	// 	}
	//
	// }

	if (g_ShowMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}
    

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}

// Load a config file name chairloaderconfig.xml.

HRESULT GetDeviceAndCtxFromSwapchain(IDXGISwapChain* pSwapChain, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
	HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)ppDevice);

	if (SUCCEEDED(ret))
		(*ppDevice)->GetImmediateContext(ppContext);

	return ret;
}

HRESULT __fastcall Present(IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags)
{
    safeToEject.lock();
	if (!g_bInitialised) {
		g_PresentHooked = true;
		std::cout << "\t[+] Present Hook called by first time" << std::endl;
		if (FAILED(GetDeviceAndCtxFromSwapchain(pChain, &pDevice, &pContext)))
			return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
		pSwapChain = pChain;
		DXGI_SWAP_CHAIN_DESC sd;
		pChain->GetDesc(&sd);
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		window = sd.OutputWindow;

		//Set OriginalWndProcHandler to the Address of the Original WndProc function
		OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);


		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX11_Init(pDevice, pContext);
		ImGui::GetIO().ImeWindowHandle = window;

		ID3D11Texture2D* pBackBuffer;

		pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
		pBackBuffer->Release();

		g_bInitialised = true;
	}
	/*
	if (bLighthack)
	{
		pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRBG(100, 100, 100));
	}
	*/
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();

	ImGui::NewFrame();
	//Menu is displayed when g_ShowMenu is TRUE
	if (g_ShowMenu)
	{
		bool bShow = true;
		// ImGui::ShowDemoWindow(&bShow);
        gui->draw(&bShow);
		
	}
	ImGui::EndFrame();

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    safeToEject.unlock();
	return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
}

void detourDirectXPresent()
{
	std::cout << "[+] Calling fnIDXGISwapChainPresent Detour" << std::endl;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	// Detours the original fnIDXGISwapChainPresent with our Present
	DetourAttach(&(LPVOID&)fnIDXGISwapChainPresent, (PBYTE)Present);
	DetourTransactionCommit();
}

void retrieveValues()
{
	DWORD_PTR hDxgi = (DWORD_PTR)GetModuleHandle(L"dxgi.dll");
#if defined(ENV64BIT)
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)((DWORD_PTR)hDxgi + 0x5070);
#elif defined (ENV32BIT)
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)((DWORD_PTR)hDxgi + 0x10230);
#endif
	std::cout << "[+] Present Addr: " << std::hex << fnIDXGISwapChainPresent << std::endl;
}

void printValues()
{
	std::cout << "[+] ID3D11DeviceContext Addr: " << std::hex << pContext << std::endl;
	std::cout << "[+] ID3D11Device Addr: " << std::hex << pDevice << std::endl;
	std::cout << "[+] ID3D11RenderTargetView Addr: " << std::hex << mainRenderTargetView << std::endl;
	std::cout << "[+] IDXGISwapChain Addr: " << std::hex << pSwapChain << std::endl;
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(hwnd, uMsg, wParam, lParam); }

void GetPresent()
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 2;
	sd.BufferDesc.Height = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	D3D_FEATURE_LEVEL FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	UINT numFeatureLevelsRequested = 1;
	D3D_FEATURE_LEVEL FeatureLevelsSupported;
	HRESULT hr;
	IDXGISwapChain* swapchain = 0;
	ID3D11Device* dev = 0;
	ID3D11DeviceContext* devcon = 0;
	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&FeatureLevelsRequested,
		numFeatureLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&swapchain,
		&dev,
		&FeatureLevelsSupported,
		&devcon)))
	{
		std::cout << "[-] Failed to hook Present with VT method." << std::endl;
		return;
	}
	DWORD_PTR* pSwapChainVtable = NULL;
	pSwapChainVtable = (DWORD_PTR*)swapchain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];
	fnIDXGISwapChainPresent = (IDXGISwapChainPresent)(DWORD_PTR)pSwapChainVtable[8];
	g_PresentHooked = true;
	std::cout << "[+] Present Addr:" << fnIDXGISwapChainPresent << std::endl;
	Sleep(2000);
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
//_DEBUG;_WINDOWS;_USRDLL
//_DEBUG;PREYTESTTRAINER_EXPORTS;_WINDOWS;_USRDLL;%(PreprocessorDefinitions)
DWORD WINAPI GUIThread(HMODULE hModule) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    while (gui == nullptr) {
        // wait for chairloader to not be fucking dead
    }
    // gui object initialized
    // Open Shared Memory 
    ip::managed_shared_memory segment(ip::open_only, "ChairloaderBase");
    // Get the environment object
	std::pair<std::pair<ChairloaderUtils*, ChairloaderGui*>*, ip::managed_shared_memory::size_type> res = segment.find<std::pair<ChairloaderUtils*, ChairloaderGui*>> ("EnvInstance");

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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ChairloaderThread, hModule, 0, nullptr));
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)GUIThread, hModule, 0, nullptr));
        // CloseHandle(CreateThread(nullptr, 0 (LPTHREAD_START_ROUTINE), hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // killDetourDirectX();
        break;
    }
    return TRUE;
}

