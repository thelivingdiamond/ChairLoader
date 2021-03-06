#include <string>
#include <windows.h>
#include "mswsock_proxy.h"

namespace {
HMODULE g_CLModule = nullptr;

// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
// I hate WinAPI
std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    // Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    // Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

void LoadChairLoader()
{
    g_CLModule = LoadLibraryA("ChairLoader.dll");
    if (!g_CLModule) {
        std::string text = "Failed to load ChairLoader\n" + GetLastErrorAsString();
        MessageBoxA(nullptr, text.c_str(), "ChairLoader Loader Error", MB_OK | MB_ICONERROR);
        std::abort();
    }
}

void ShutdownChairLoader()
{
    if (g_CLModule)
    {
        FreeLibrary(g_CLModule);
        g_CLModule = nullptr;
    }
}

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MSWSProxy_Init();
        LoadChairLoader();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        ShutdownChairLoader();
        MSWSProxy_Shutdown();
        break;
    }
    return TRUE;
}

