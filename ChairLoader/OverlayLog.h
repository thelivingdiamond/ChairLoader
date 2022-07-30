#pragma once
#include "pch.h"
#include <Prey/ArkEntityArchetypeLibrary.h>
#include <ChairLoader/IChairloaderGui.h>

class ChairloaderGUILog {
public:
    // Basic Logging Structs

    ChairloaderGUILog();
    ~ChairloaderGUILog();
    // show persistent transparent log overlay
    void drawDisplay();
    // Draw the log history window to display all previous log messages
    void drawHistory(bool* bShow);
    // Log Item with std::string, modName, Level, and screenWriting
    void logItem(std::string msg, const std::string modNameIn, logLevel level = logLevel::normal, bool displayToScreen = true);
    // Log Item with a premade logMessage
    void logItem(logMessage message, bool displayToScreen = true);

private:
    const std::string modName = "OverlayLog";
    std::vector<logMessage> archiveLogQueue;
    std::vector<logMessage> displayLogQueue;
    time_t MessageTimeoutTime = 10;
};

extern ChairloaderGUILog* GUILog;