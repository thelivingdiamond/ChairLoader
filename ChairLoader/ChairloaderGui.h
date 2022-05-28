#pragma once


#include "ArkBasicTypes.h"
#include "ChairloaderUtils.h"
#include "StdAfx.h"
#include "preyDllObjects.h"
#include "ArkEntityArchetypeLibrary.h"
// #include <stack>
//TODO: INCLUDE LISTENER/HANDLER FUNCTIONS
class ChairloaderGui {
public:
    enum class logLevel {
        normal = 0,
        warning = 1,
        error = 2,
    };
    struct logMessage {
        std::string message;
    	std::string modName;
        time_t time;
        logLevel level;

        bool empty() {
            return message.empty();
        }
    };
private:
    ChairloaderUtils* chairloaderGlobal;
    const std::string modName = "ChrldrGui";
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
    // chairloaderGuiControl control;
    
    static void RightAlignText(const char* text) {
        auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x
            - ImGui::GetScrollX());
        if (posX > ImGui::GetCursorPosX())
            ImGui::SetCursorPosX(posX);
    }
    static void HelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    class chairloaderGuiLog {
    public:
        //Draw Display
        
        void drawDisplay() {
            // show persistent transparent log overlay
            // TODO: make it grow from the bottom
            if (!displayLogQueue.empty()) {
                ImGui::SetNextWindowPos(ImVec2(5.0f, 20.0f));
                ImGui::SetNextWindowSize(ImVec2(700, 150));
                if (ImGui::Begin("Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground)) {
                    float nextCursorPos = ImGui::GetContentRegionAvail().y;
                    // std::vector::reverse_iterator
                    for (auto itr = displayLogQueue.rbegin(); itr != displayLogQueue.rend(); ++itr) {
                        
                        if (time(nullptr) >= itr->time + MessageTimeoutTime) {
                            std::advance(itr, 1);
                            displayLogQueue.erase(itr.base());
                            break;
                        }
                        else {
                            nextCursorPos -= ImGui::CalcTextSize(itr->message.c_str(), nullptr, false, ImGui::GetContentRegionAvail().x).y;
                            ImGui::SetCursorPosY(nextCursorPos);
                            if (itr->level == logLevel::normal) {
                                ImGui::TextWrapped("%s", itr->message.c_str());
                            }
                            else if (itr->level == logLevel::warning) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1,1,0,1 });
                                ImGui::TextWrapped("%s", itr->message.c_str());
                                ImGui::PopStyleColor();
                            }
                            else if (itr->level == logLevel::error) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1,0,0,1 });
                                ImGui::TextWrapped("%s", itr->message.c_str());
                                ImGui::PopStyleColor();
                            }
                            // nextCursorPos -= ImGui::GetItemRectSize().y;
                            if (nextCursorPos < 0)
                                break;
                        }
                    }
                    ImGui::End();
                }
            }
        }
        void drawHistory(bool* bShow) {
            // show log history
            if (ImGui::Begin("Log History", bShow)) {
                if (ImGui::BeginTable("Log Table", 2, ImGuiTableFlags_SizingFixedFit)) {
                    ImGui::TableSetupColumn("Time");
                    ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableHeadersRow();

                    for (auto itr = archiveLogQueue.rbegin(); itr != archiveLogQueue.rend(); ++itr) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::TextWrapped("%s", itr->modName);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Bullet();
                        ImGui::SameLine();
                        if (itr->level == logLevel::normal) {
                            ImGui::TextWrapped("%s", itr->message.c_str());
                        }
                        else if (itr->level == logLevel::warning) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1,1,0,1 });
                            ImGui::TextWrapped("%s", itr->message.c_str());
                            ImGui::PopStyleColor();
                        }
                        else if (itr->level == logLevel::error) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1,0,0,1 });
                            ImGui::TextWrapped("%s", itr->message.c_str());
                            ImGui::PopStyleColor();
                        }
                    }
                    ImGui::EndTable();
                }
                ImGui::End();
            }
        }
        
        std::function<void()> invokeDisplayDraw = std::bind(&chairloaderGuiLog::drawDisplay, this);

        // Logging Function
        void logItem(std::string msg, const std::string modName, logLevel level = logLevel::normal, bool displayToScreen = true) {
                logMessage message = { msg, modName, time(nullptr), level};
                archiveLogQueue.emplace_back(message);
                if (displayToScreen)
                    displayLogQueue.emplace_back(message);
        }
        void logItem(logMessage message, bool displayToScreen = true) {
            if (!message.empty()) {
                archiveLogQueue.emplace_back(message);
                if (displayToScreen)
                    displayLogQueue.emplace_back(message);
            }
        }
        time_t MessageTimeoutTime = 10;
    private:
        const std::string modName = "ChrldrGui.Log";
        std::vector<logMessage> archiveLogQueue;
        std::vector<logMessage> displayLogQueue;
    };

    class chairloaderGuiEntityManager {
    private:
        const std::string modName = "ChrldrGui.EntityManager";
        std::vector<CEntity*> entityDisplayList;
        bool refreshDisplayList = true;
        std::string filterText, oldFilterText;
        CEntity* selected = nullptr;
    public:
        enum class entityModifyType {
            none = -1,
	        pos = 0,
            rot = 1,
            scale = 2,
            playerPos = 3,
        };
        struct entityModifyRequest {
            CEntity* entity;
            entityModifyType type;
            Vec3_tpl<float> pos {0,0,0};
            Vec4_tpl<float> rot{ 0,0,0,0 };
            Vec3_tpl<float> scale = { 0,0,0 };
        };
        std::queue<entityModifyRequest> modifyQueue;

        chairloaderGuiEntityManager(ChairloaderUtils* chairloader) {
            archetypeToSpawn = nullptr;
            archetypeFilterRequestQueue.push(archetypeFilterRequest{ "" });
        }


        // archetype spawns
        struct spawnRequest {
            CEntityArchetype* archetype;
            std::string name;
            Vec3_tpl<float> pos = {0,0,0};
            Vec4_tpl<float> rot = { 0,0,0,1 };
            int spawnCount = 1;
            bool usePlayerPos = true,
                offsetFromPlayer = true;
        };
        std::queue<spawnRequest> archetypeSpawnRequestQueue;
        struct archetypeFilterRequest {
            std::string text;
        };
        CEntityArchetype* archetypeToSpawn;

        // archetype filter
        std::string archetypeFilterText, oldArchetypeFilterText;
        std::queue<archetypeFilterRequest> archetypeFilterRequestQueue;
        std::vector<CEntityArchetype*> archetypeFilteredList;
        
        void draw(bool* bShow) {
            if (!ImGui::Begin("Entity Manager", bShow)) {
                ImGui::End();
                return;
            }
        	if (ImGui::BeginTabBar("Entity Bar")) {
                if (ImGui::BeginTabItem("Entity Spawner")) {

                    // Helper class to easy setup a text filter.
                    // You may want to implement a more feature-full filtering scheme in your own application.
                    static bool usePlayerPos, offsetFromPlayer;
                    static std::string inputId, inputName;
                    static std::string statusMessage;
                    static float spawnX, spawnY, spawnZ;
                    static ImVec4 color{ 1,1,1,1 };
                    static time_t statusTimer;
                    static int spawnCount{ 1 };
                    ImGui::TextColored(color, statusMessage.c_str());
                    if (archetypeToSpawn == nullptr) {
                        ImGui::Text("Entity Archetype Name: ");
                        ImGui::Text("Entity ID: ");

                    }
                    else {
                        ImGui::Text("Entity Archetype Name: %s", archetypeToSpawn->m_name.m_str);
                        ImGui::Text("Entity ID: %d", archetypeToSpawn->m_id);
                    }
                    ImGui::InputText("Entity Name", &inputName, ImGuiInputTextFlags_None);
                    ImGui::InputInt("Spawn Count", &spawnCount);
                    ImGui::Checkbox("Use Player Pos", &usePlayerPos);
                    if (usePlayerPos) {
                        ImGui::Checkbox("Offset In front of Player", &offsetFromPlayer);
                    }
                    else {
                        ImGui::InputFloat("X", &spawnX);
                        ImGui::InputFloat("Y", &spawnY);
                        ImGui::InputFloat("Z", &spawnZ);
                    }

                    if (ImGui::Button("Spawn Entity")) {
                        //TODO: make it spawn an entity when the button is pushed
                        try {
                            if (archetypeToSpawn != nullptr) {
                                if (inputName != "") {
                                    spawnRequest request;
                                    request.archetype = archetypeToSpawn;
                                    request.usePlayerPos = usePlayerPos;
                                    request.offsetFromPlayer = offsetFromPlayer;
                                    request.name = inputName;
                                    if (spawnCount >= 1) {
                                        request.spawnCount = spawnCount;
                                    }
                                    else {
                                        if (spawnCount > 999) {
                                            throw("Error, invalid spawn count (spawn count too high)");
                                        }

                                        throw("Error, invalid spawn count");
                                    }
                                    request.pos.x = spawnX;
                                    request.pos.y = spawnY;
                                    request.pos.z = spawnZ;
                                    archetypeSpawnRequestQueue.push(request);
                                    // done
                                    color = { 1,1,1,1 };
                                    statusMessage = "spawned an entity: " + inputName;
                                    time(&statusTimer);
                                }
                                else {
                                    throw ("Error, invalid entity name");
                                }
                            }
                            else {
                                throw("Error, no archetype selected");
                            }
                        }
                        catch (const char* c) {
                            color = { 1,0,0,1 };
                            statusMessage = c;
                            time(&statusTimer);
                        }
                    }
                    ImGui::Text("Filter:");
                    ImGui::InputText("", &archetypeFilterText);
                    if (archetypeFilterText != oldArchetypeFilterText) {
                        oldArchetypeFilterText = archetypeFilterText;
                        archetypeFilterRequestQueue.push(archetypeFilterRequest{ archetypeFilterText });
                        // printf("requested: %s\n", archetypeFilterText.c_str());
                    }
                    // filter.Draw(" ");
                    ImGui::SameLine();
                    HelpMarker("Filter usage:\n"
                        "  \"\"         display all lines\n"
                        "  \"xxx\"      display lines containing \"xxx\"\n"
                        "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
                        "  \"-xxx\"     hide lines containing \"xxx\"");
                    //TODO: Replace this with actual archetype list, potentially with custom struct for easy sorting
                    static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable;
                    ImVec2 outer_size = ImVec2(0.0f, ImGui::CalcTextSize("A").x * 16.0f);
                    if (ImGui::BeginTable("3ways", 2, flags, outer_size)) {

                        ImGui::TableSetupScrollFreeze(0, 1);
                        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
                        // if (ImGui::IsItemHovered())
                        //     ImGui::SetTooltip("Click to view archetype information");
                        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None);
                        // if (ImGui::IsItemHovered())
                        //     ImGui::SetTooltip("Click to select archetype");
                        ImGui::TableHeadersRow();
                        //TODO: probably implement clipper
                        // if filter changes
                        // filter through all archetypes
                        // if filter is empty or smaller than x characters do not update




                        // ImGuiListClipper clipper;
                        // clipper.Begin(200);
                        // while (clipper.Step()) {
                        auto itr = archetypeFilteredList.begin();
                        for (int clip = 0; clip < 200 && itr != archetypeFilteredList.end(); clip++, ++itr) {
                            static bool nameClicked, idClicked;
                            CEntityArchetype* archetype = *itr;
                            ImGui::TableNextRow();
                            //TODO: Context popup for archetype info?
                            //TODO: Pull information out of archetypes
                            ImGui::TableSetColumnIndex(0);

                            nameClicked = ImGui::Selectable(archetype->m_name.m_str);
                            // printf("Name Printed\n");
                            if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonLeft)) // <-- use last item id as popup id
                            {
                                ImGui::Text("Display information about the archetype here");
                                if (ImGui::Button("Choose"))
                                    // inputId = std::to_string(archetype->m_id);
                                    archetypeToSpawn = archetype;
                                if (ImGui::Button("Close"))
                                    ImGui::CloseCurrentPopup();
                                ImGui::EndPopup();
                            }

                            ImGui::TableSetColumnIndex(1);
                            if (ImGui::Selectable(std::to_string(archetype->m_id).c_str())) {
                                // inputId = std::to_string(archetype->m_id);
                                archetypeToSpawn = archetype;
                            }
                            // printf("id Printed\n");
                        }
                        // }
                        if (time(NULL) > statusTimer + 3)
                            statusMessage = "";
                        // }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Entity List")) {
                    {
                        
                        ImGui::BeginChild("left pane", ImVec2(250, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
                        ImGui::InputText("", &filterText);
                        if(oldFilterText != filterText) {
                            oldFilterText = filterText;
                            refreshDisplayList = true;
                        }
                        auto itr = entityDisplayList.begin();
                        for (int i = 0; itr != entityDisplayList.end() && i < 500; i++, ++itr) {
                            // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                            std::string label;
                            if((*itr)->m_szName.m_str != nullptr) {
                                label = (*itr)->m_szName.m_str;
                            } else {
                                label = "invalid name";
                            }
                            // sprintf_s(label, "Entity: %d", i);
                            if (ImGui::Selectable(label.c_str(), selected == (*itr)))
                                selected = (*itr);
                        }
                        ImGui::EndChild();
                    }
                    ImGui::SameLine();

                    // Right
                    {
                        ImGui::BeginGroup();
                        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                        if (selected != nullptr) {
                            ImGui::Text("Entity: %llu", selected->m_nID);
                        } else {
                            ImGui::Text("Entity: %s", "Null");
                        }
                        ImGui::Separator();
                        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton))
                        {
                            if (ImGui::BeginTabItem("Entity Details"))
                            {
                                // printf("at the entity check\n");
                                
                                if (selected != nullptr) {
                                    if (ImGui::BeginTable("Details", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_BordersH)) {
                                        // Setup Columns
                                        ImGui::TableSetupColumn(" Item:", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                                        ImGui::TableSetupColumn(" Value:");
                                        // Name
                                        ImGui::TableNextRow();  
                                        ImGui::TableNextColumn();
                                        RightAlignText("Name:");
                                        ImGui::Text("Name:");
                                        ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                                        ImGui::TableNextColumn();
                                        // printf("at name\n");
                                        if (selected->m_szName.m_str != nullptr)
                                            ImGui::Text("%s", selected->m_szName.m_str);
                                        else
                                            ImGui::Text("%s", "--Null Name--");
                                        // ID
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        RightAlignText("ID:");
                                        ImGui::Text("ID:");
                                        ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                                        ImGui::TableNextColumn();
                                        // printf("at id\n");
                                        ImGui::Text("%llu", selected->m_nID);
                                        // GUID
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        // printf("at guid\n");
                                        RightAlignText("GUID:");
                                        ImGui::Text("GUID:");
                                        ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%llu", selected->m_guid);
                                        // Class
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        // printf("at class\n");
                                        RightAlignText("Class:");
                                        ImGui::Text("Class:");
                                        ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                                        ImGui::TableNextColumn();
                                        if (selected->m_pClass != nullptr) {
                                            if (((CEntityClass*)selected->m_pClass)->m_sName.m_str != nullptr)
                                                ImGui::Text("%s", ((CEntityClass*)selected->m_pClass)->m_sName.m_str);
                                            else
                                                ImGui::Text("%s", "--Null Class--");
                                        }
                                        else {
                                            ImGui::Text("%s", "--Null Class--");
                                        }
                                        // Archetype
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        // printf("at archetype\n");
                                        RightAlignText("Archetype:");
                                        ImGui::Text("Archetype:");
                                        ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                                        ImGui::TableNextColumn();
                                        if (selected->m_pArchetype != nullptr) {
                                            if (selected->m_pArchetype->m_name.m_str != nullptr)
                                                ImGui::Text("%s", selected->m_pArchetype->m_name.m_str);
                                            else
                                                ImGui::Text("%s", "--Null Archetype--");
                                        }
                                        else {
                                            ImGui::Text("%s", "--Null Archetype--");
                                        }

                                        ImGui::EndTable();
                                    }
                                    static float pos[3];
                                    pos[0] = selected->m_vPos.x;
                                    pos[1] = selected->m_vPos.y;
                                    pos[2] = selected->m_vPos.z;
                                    static float newPos[3];
                                    static float rot[4];
                                    rot[0] = selected->m_qRotation.x;
                                    rot[1] = selected->m_qRotation.y;
                                    rot[2] = selected->m_qRotation.z;
                                    rot[2] = selected->m_qRotation.w;
                                    static float newRot[3];
                                    static float scale[3]{ 1,1,1 };
                                    scale[0] = selected->m_vScale.x;
                                    scale[1] = selected->m_vScale.y;
                                    scale[2] = selected->m_vScale.z;
                                    static float newScale[3];
                                    // static float x, y, z, rx, rz, ry, rw = 1;
                                    ImGui::Text("Position: ");
                                    ImGui::InputFloat3("pos", pos, "%.1f", ImGuiInputTextFlags_ReadOnly);

                                    ImGui::Text("Rotation: ");
                                    ImGui::InputFloat4("rot", rot, "%.1f", ImGuiInputTextFlags_ReadOnly);

                                    ImGui::Text("Scale: ");
                                    ImGui::InputFloat3("scale", scale, "%.1f", ImGuiInputTextFlags_ReadOnly);
                                    ImGui::NewLine();
                                    ImGui::Button("Set Pos");
                                    if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonLeft)) {
                                        static bool playerPos = false;
                                        if(!playerPos)
											ImGui::InputFloat3("pos", newPos, "%.2f");
                                        ImGui::Checkbox("Set To Player Pos", &playerPos);
                                        if (ImGui::Button("Set")) {
                                            if(playerPos)
                                                modifyQueue.push(entityModifyRequest{ selected, entityModifyType::playerPos});
                                            
                                            else
												modifyQueue.push(entityModifyRequest{ selected, entityModifyType::pos, Vec3_tpl<float>{newPos[0], newPos[1], newPos[2]} });
                                        }
                                        ImGui::EndPopup();
                                    }

                                    ImGui::SameLine();
                                    ImGui::Button("Set Rot");
                                    if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonLeft)) {
                                        ImGui::InputFloat4("rot", newRot, "%.2f");
                                        ImGui::Button("Set");
                                        ImGui::EndPopup();
                                    }

                                    ImGui::SameLine();
                                    ImGui::Button("Set Scale");
                                    if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonLeft)) {
                                        newScale[1] = newScale[0];
                                        newScale[2] = newScale[0];
                                        ImGui::InputFloat3("scale", newScale, "%.2f");
                                        if(ImGui::Button("Set"))
                                            modifyQueue.push(entityModifyRequest{ selected, entityModifyType::scale, Vec3_tpl<float>{newScale[0], newScale[0], newScale[0]} });
                                        ImGui::EndPopup();
                                    }
                                    // ImGui::Text("Rotation");

                                    // ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                                    // ImGui::Text("Pos:\nx:%f\ny:%f\nz:%f", 1.0f, 2.0f, 420.0f);
                                }
                                else {
                                    ImGui::Text("No Entity Selected");
                                }
                                ImGui::EndTabItem();
                            }

                            if (ImGui::BeginTabItem("Status Effects"))
                            {
                                static std::string selectedStatus;
                                // if(ImGui::BeginChild("status list", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.6f))){
                                if (ImGui::BeginTable("statusList", 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter, ImVec2(0, ImGui::GetContentRegionAvail().y * 0.8f))) {
                                    ImGui::TableSetupColumn("Status");
                                    ImGui::TableHeadersRow();
                                    for (auto itr = statusPush.begin(); itr != statusPush.end(); ++itr) {
                                        ImGui::TableNextRow();
                                        // if(ImGui::TreeNode((itr)->c_str())) {
                                        // ImGui::Text("%s", itr->c_str());
                                        ImGui::TableSetColumnIndex(0);
                                        if (ImGui::Selectable(itr->c_str(), *itr == selectedStatus)) {
                                            selectedStatus = *itr;
                                        }
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::Text("%s", selectedStatus.c_str());
                                if (std::find(statusPush.begin(), statusPush.end(), selectedStatus) != statusPush.end()) {
                                    // ImGui::SameLine();
                                    ImGui::Button("Push");
                                }
                                if (std::find(statusPop.begin(), statusPop.end(), selectedStatus) != statusPop.end()) {
                                    ImGui::SameLine();
                                    ImGui::Button("Pop");
                                }
                                // ImGui::Text("ID: 0123456789");
                                ImGui::EndTabItem();
                            }
                            if (ImGui::BeginTabItem("Npc Details")) {
                                if (ImGui::BeginTable("SettableGettables", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersInnerV)) {
                                    ImGui::TableSetupColumn("Readable", ImGuiTableColumnFlags_WidthFixed, ImGui::GetContentRegionAvail().x * 0.5f);
                                    ImGui::TableSetupColumn("Writable", ImGuiTableColumnFlags_WidthFixed,
                                        ImGui::GetContentRegionAvail().x * 0.5f);
                                    ImGui::TableSetupScrollFreeze(0, 1);
                                    ImGui::TableHeadersRow();
                                    auto itrget = get.begin();
                                    auto itrset = set.begin();
                                    bool setend = false, getend = false;
                                    while (itrset != set.end() || itrget != get.end()) {
                                        ImGui::TableNextRow();
                                        if (!getend) {
                                            ++itrget;
                                            if (itrget == get.end()) {
                                                getend = true;
                                            }
                                        }
                                        if (!getend) {
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("%s", itrget->c_str());
                                            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 10.0f);
                                            std::string text = "0";
                                            ImGui::InputText("", &text, ImGuiInputTextFlags_ReadOnly);
                                            ImGui::PopItemWidth();
                                        }
                                        else {

                                        }
                                        if (!setend) {
                                            ++itrset;
                                            if (itrset == set.end()) {
                                                setend = true;
                                            }
                                        }
                                        if (!setend) {
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("%s", itrset->c_str());
                                            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(" Set   ").x);
                                            std::string text = "0";
                                            ImGui::InputText("", &text, ImGuiInputTextFlags_ReadOnly);
                                            ImGui::SameLine();
                                            ImGui::Button("Set");
                                            ImGui::PopItemWidth();
                                        }
                                        else {

                                        }
                                    }
                                    ImGui::EndTable();
                                }
                                ImGui::EndTabItem();
                            }
                            if (ImGui::BeginTabItem("Npc Info")) {
                                if (ImGui::BeginTable("IsTable", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersH | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail())) {
                                    ImGui::TableSetupColumn("Param");
                                    ImGui::TableSetupColumn("Value");
                                    for (auto itr = is.begin(); itr != is.end(); ++itr) {
                                        ImGui::TableNextRow();
                                        ImGui::TableSetColumnIndex(0);
                                        // RightAlignText(itr->c_str());
                                        ImGui::Text("%s", itr->c_str());
                                        // ImGui::SetCursorPosX(ImGui::GetColumnWidth(1));
                                        ImGui::TableSetColumnIndex(1);
                                        ImGui::Text("True");
                                    }
                                    ImGui::EndTable();
                                }
                                // ImGui::ListBox("")
                                ImGui::EndTabItem();
                            }
                            if (ImGui::BeginTabItem("Actions:")) {

                                ImGui::EndTabItem();
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndChild();
                        if (ImGui::Button("Revert")) {}
                        ImGui::SameLine();
                        if (ImGui::Button("Save")) {}
                        ImGui::EndGroup();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
            // }
        }
        void drawMenuBar(chairloaderGuiControl* control, ChairloaderUtils* chairloader) {
            if (ImGui::BeginMenu("Entity")) {
                std::unordered_map< uint64_t, CEntityArchetype*>* archetypeList = &chairloader->preyEnvironmentPointers->pEntitySystem->m_pEntityArchetypeManager->m_idToArchetypeMap;
                if (ImGui::BeginMenu("Spawn Entity")) {
                    ImGui::MenuItem("Spawn Last Spawned", nullptr, false, archetypeToSpawn != nullptr);
                    ImGui::Separator();
                    ImGui::TextDisabled("Quick Spawns");
                    if (ImGui::BeginMenu("Spawn Typhon")) {
                       
                        if (ImGui::BeginMenu("Mimics")) {
                            if(ImGui::MenuItem("Mimic"))
								archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Mimics.Mimic)->second });
                            if (ImGui::MenuItem("Greater Mimic"))
								archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Mimics.EliteMimic)->second });
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Phantoms")) {
                            if(ImGui::MenuItem("Normal"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Phantoms.BasePhantom)->second });
                            if(ImGui::MenuItem("Thermal"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Phantoms.ThermalPhantom)->second });
                            if(ImGui::MenuItem("Etheric"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Phantoms.EthericPhantom)->second });
                            if(ImGui::MenuItem("Voltaic"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Phantoms.VoltaicPhantom)->second });
                            ImGui::EndMenu();
                        }
                        if(ImGui::MenuItem("Nightmare"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.ArkNightmare)->second });
                        if(ImGui::MenuItem("Poltergeist"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.ArkPoltergeist)->second });
                        if(ImGui::MenuItem("Telepath"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Overseers.Telepath)->second });
                        if(ImGui::MenuItem("Technopath"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkNpcs.Overseers.Technopath)->second });
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Spawn Operator")) {
                        if(ImGui::MenuItem("Medical"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkRobots.Operators.Generic.MedicalOperator)->second });
                        if(ImGui::MenuItem("Engineering"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkRobots.Operators.Generic.EngineeringOperator)->second });
                        if(ImGui::MenuItem("Science"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkRobots.Operators.Generic.ScienceOperator)->second });
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Spawn Turrets")) {
                        if (ImGui::MenuItem("Turret"))
                            archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkRobots.Turrets.Turret_Default)->second });
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 1,1,1,0.5f });
                    ImGui::TextWrapped("Item Spawning Not Stable, use at own risk");
                    ImGui::PopStyleColor();
                    if (ImGui::BeginMenu("Spawn Items")) {
                        if (ImGui::BeginMenu("Weapons")) {
                            if(ImGui::MenuItem("Shotgun"))
								archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Weapons.Shotgun)->second });
                            if(ImGui::MenuItem("Shotgun Ammo"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Ammo.ShotgunShells)->second });
                            if(ImGui::MenuItem("Pistol"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Weapons.Pistol)->second });
                            if(ImGui::MenuItem("Pistol Ammo"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Ammo.PistolBullets)->second });
                            if(ImGui::MenuItem("Gloo Gun"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Weapons.GooGun)->second });
                            if(ImGui::MenuItem("Gloo Gun Ammo"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Ammo.GooGun)->second });
                            if(ImGui::MenuItem("Q Beam"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Weapons.Instalaser)->second });
                            if(ImGui::MenuItem("Q Beam Ammo"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Ammo.InstaLaser)->second });
                            if(ImGui::MenuItem("Stun Gun"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Weapons.StunGun)->second });
                            if(ImGui::MenuItem("Stun Gun Ammo"))
                                archetypeSpawnRequestQueue.push(spawnRequest{ archetypeList->find(entityArchetypeLibrary.ArkPickups.Ammo.StunGunAmmo)->second });
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    // if(ImGui::BeginMenu("Spawn Human")) {
                    //     ImGui::MenuItem("Marco Simmons");
                    //     ImGui::MenuItem("Igwe");
                    //     ImGui::MenuItem("Mikhaila");
                    //     ImGui::MenuItem("");
                    //     ImGui::EndMenu();
                    // }
                    ImGui::EndMenu();
                }
                ImGui::MenuItem("Show Entity Manager", nullptr, &control->showEntityManager);
                ImGui::EndMenu();
            }
        }
        void entityModifyHandler(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            try {
                if (!modifyQueue.empty()) {
                    entityModifyRequest request = modifyQueue.front();
                    if(request.entity == nullptr) {
                        throw("Null Entity");
                    }
                    modifyQueue.pop();
                    if (request.type == entityModifyType::pos) {
                        if (request.pos.x != 0 && request.pos.y != 0 && request.pos.z != 0) {
                            ((IEntity*)request.entity)->SetPos(&request.pos, 0, false, false);
                            log->logItem("set entity pos", modName);
                        } else {
                            throw("Null Pos");
                        }
                    } if (request.type == entityModifyType::scale) {
                        if ((request.scale.x != 0 && request.scale.y != 0 && request.scale.z != 0) && (request.scale.x == request.scale.y && request.scale.y == request.scale.z) ) {
                            ((IEntity*)request.entity)->SetScale(&request.scale, 0);
                        }
                        else {
                            throw("Null Scale");
                        }
                    } if (request.type == entityModifyType::playerPos) {
	                    if(chairloader->ArkPlayerPtr != nullptr) {
                            Vec3_tpl<float> pos;
                            chairloader->internalPreyFunctions->ArkPlayerF->getPlayerWorldEyePos(chairloader->ArkPlayerPtr, &pos);
                            ((IEntity*)request.entity)->SetPos(&pos, 0, false, false);
                            log->logItem("set entity pos to player pos", modName);
	                    }
                    }
                }
            }
            catch (char* c) {
                std::string msg = "Error handling entity modification request: ";
                log->logItem(msg + c, modName,logLevel::error);
            }
        }

        void filterEntityList(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            if (refreshDisplayList) {
                selected == nullptr;
                entityDisplayList.clear();
                int i = 0;
                
                for (auto itr = chairloader->preyEnvironmentPointers->pEntitySystem->m_EntityArray.begin(); itr != chairloader->preyEnvironmentPointers->pEntitySystem->m_EntityArray.end(); ++itr) {
                    if (*itr != nullptr) {
                        if((*itr)->m_szName.m_str != nullptr) {
                            std::string name = (*itr)->m_szName.m_str;
                            std::string newFilterText = filterText;
                            std::transform(newFilterText.begin(), newFilterText.end(), newFilterText.begin(), ::tolower);
                            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                            if(name.find(newFilterText) != name.npos || filterText.empty()) {
                                entityDisplayList.emplace_back(*itr);
                            }
                        }
                    }
                }
                refreshDisplayList = false;
            }
        }
        void archetypeSpawnRequestHandler(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            if (!archetypeSpawnRequestQueue.empty() && !chairloader->preyEnvironmentPointers->pSystem->IsPaused()) {
                try {
                    
                    spawnRequest request = archetypeSpawnRequestQueue.front();
                    archetypeSpawnRequestQueue.pop();
                    std::string archetypeName = request.archetype->m_name.m_str;

                    // chairloader->entityArchetypeLibrary.
                    if (request.usePlayerPos) {
                        Vec3_tpl<float> playerPos;
                        ArkPlayer* player = chairloader->internalPreyFunctions->ArkPlayerF->getInstance();
                        chairloader->internalPreyFunctions->ArkPlayerF->getPlayerWorldEyePos(player, &playerPos);
                        if (player != nullptr) {
                            // player->GetLocalEyePos(&playerPos);
                            // printf("Player Position x: %f y: %f z:%f\n", playerPos.x, playerPos.y, playerPos.z);
                            if (playerPos.x != 0 && playerPos.y != 0 && playerPos.z != 0) {
                                if (request.offsetFromPlayer) {
                                    playerPos.x += chairloader->ArkPlayerPtr->m_cachedReticleDir.x * 5;
                                    playerPos.y += chairloader->ArkPlayerPtr->m_cachedReticleDir.y * 5;
                                }
                                request.pos = playerPos;
                            }
                            else {
                                throw ("Error, null player position");
                            }
                        }
                        else {
                            throw("Error, null player pointer");
                        }
                    }
                    if ((archetypeName.find("ArkRobots") != std::string::npos || archetypeName.find("ArkHumans") != std::string::npos || archetypeName.find("ArkNpcs") != std::string::npos) && archetypeName != "Turrets.Turret_Default") {
                        if (request.spawnCount == 1) {
                            IEntity* spawnerEntity = chairloader->spawnerHelper->getVictimSpawnerEntity(ChairloaderUtils::EntityType::mimic);
                            if (spawnerEntity != nullptr) {
                                char* oldArchetypeName = chairloader->spawnerHelper->setEntityArchetype(request.archetype->m_id, spawnerEntity);
                                if (oldArchetypeName != nullptr) {
                                    CArkNpcSpawner* spawner = chairloader->internalPreyFunctions->CEntity->getArkNpcSpawner((CEntity*)spawnerEntity);
                                    IEntity* newEntity = chairloader->spawnerHelper->spawnNpc(spawner, (char*)request.name.c_str());
                                    // printf("spawned an entity\n");
                                    if (newEntity != nullptr) {
                                        newEntity->SetPos(&request.pos, 0, true, false);
                                        newEntity->SetName((char*)request.name.c_str());
                                        // printf("set position of an entity to x: %f y: %f z:%f\n", request.pos.x, request.pos.y, request.pos.z);
                                    }
                                    else {
                                        throw("Error, null entity spawned");
                                    }
                                    chairloader->spawnerHelper->setEntityArchetype(oldArchetypeName, spawnerEntity);
                                    log->logItem( "Spawned entity successfully", modName);
                                }
                                else {
                                    throw("Error, Old archetype Was null (single spawn)");
                                }
                            }
                            else {
                                throw("Error, spawner Entity Was null (single spawn)");
                            }
                        }
                        else {
                            IEntity* spawnerEntity = chairloader->spawnerHelper->getVictimSpawnerEntity(ChairloaderUtils::EntityType::mimic);
                            if (spawnerEntity != nullptr) {
                                char* oldArchetypeName = chairloader->spawnerHelper->setEntityArchetype(request.archetype->m_id, spawnerEntity);
                                if (oldArchetypeName != nullptr) {
                                    CArkNpcSpawner* spawner = chairloader->internalPreyFunctions->CEntity->getArkNpcSpawner((CEntity*)spawnerEntity);
                                    for (int i = 0; i < request.spawnCount; i++) {
                                        IEntity* newEntity = chairloader->spawnerHelper->spawnNpc(spawner, (char*)request.name.c_str());
                                        if (newEntity != nullptr) {
                                            newEntity->SetPos(&request.pos, 0, true, false);
            //                                 if(!request.name.empty())
												// newEntity->SetName((char*)(request.name + std::to_string(i)).c_str());
                                            // printf("set position of an entity to x: %f y: %f z:%f\n", request.pos.x, request.pos.y, request.pos.z);
                                        }
                                        else {
                                            throw("Error, null entity spawned");
                                        }
                                        Sleep(10);
                                    }
                                    chairloader->spawnerHelper->setEntityArchetype(oldArchetypeName, spawnerEntity);
                                    log->logItem("Spawned entity successfully", modName);
                                    // return logMessage{ , time(nullptr), logLevel::normal };
                                }
                                else {
                                    throw("Error, Old archetype Was null (single spawn)");
                                }
                            }
                            else {
                                throw("Error, spawner Entity Was null (single spawn)");
                            }
                        }
                    }
                    else {
                        log->logItem("Using non - npc spawning process", modName);
                        // printf("Using non-npc spawning process\n");
                        SEntitySpawnParams* params = new SEntitySpawnParams;

                        params->vScale.x = 1;
                        params->vScale.y = 1;
                        params->vScale.z = 1;
                        uint32_t id = chairloader->internalPreyFunctions->CEntitySystemF->generateEntityId(chairloader->preyEnvironmentPointers->pEntitySystem, true);
                        params->id = id;
                        params->vPosition.x = request.pos.x;
                        params->vPosition.y = request.pos.y;
                        params->vPosition.z = request.pos.z;
                        params->qRotation.x = request.rot.x;
                        params->qRotation.y = request.rot.y;
                        params->qRotation.z = request.rot.z;
                        params->qRotation.w = request.rot.w;
                        params->sLayerName = (char*)"";
                        params->pClass = (CEntityClass*)(0x0);
                        params->pArchetype = (CEntityArchetype*)(0x0);
                        params->guid = 0;
                        params->prevGuid = 0;
                        params->prevId = 0;
                        params->bCreatedThroughPool = 0;
                        params->bIgnoreLock = 0;
                        params->bStaticEntityId = 0;
                        params->nFlags = 0;
                        params->nFlagsExtended = 0;
                        params->sName = (char*)request.name.c_str();
                        params->entityNode.ptr = (IXmlNode*)0x0;
                        params->shadowCasterType = '\0';
                        params->pUserData = (void*)0x0;
                        params->sceneMask = '\0';
                        IEntity* entity;

                        // IEntityArchetype* archetype = (IEntityArchetype*)chairloader->preyEnvironmentPointers->pEntitySystem->GetEntityArchetype();
                        if (request.archetype != nullptr) {
                            entity = chairloader->preyEnvironmentPointers->pEntitySystem->SpawnEntityFromArchetype((IEntityArchetype*)request.archetype, params, true);
                        }
                        else {
                            throw("Error, no archetype found\n");
                        }




                    }
                }
                catch (const char* c) {
                    log->logItem(c, modName,logLevel::error);
                    // return {c, time(nullptr), logLevel::error}
                    // printf("%s\n", c);
                }
            }
        }
        void archetypeFilterRequestHandler(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            if (!archetypeFilterRequestQueue.empty()) {
                archetypeFilterRequest request = archetypeFilterRequestQueue.front();
                std::string filterText = request.text;
                std::map<const char*, CEntityArchetype*>* archetypeList = &chairloader->preyEnvironmentPointers->pEntitySystem->m_pEntityArchetypeManager->m_nameToArchetypeMap;
                // static ImGuiTextFilter filter;
                auto itr = archetypeList->begin();
                archetypeFilteredList.clear();
                // if (filterText.size() < strlen(filter.InputBuf)) {
                // strcpy_s(filter.InputBuf, filterText.size(), filterText.c_str());
                // }
                for (int i = 0; i < 400 && itr != archetypeList->end(); ++itr) {
                    std::string archetypeName = itr->first;
                    // size_t last_period = archetypeName.find_last_of('.');
                    // archetypeName = archetypeName.substr(last_period, archetypeName.size());
                    std::transform(archetypeName.begin(), archetypeName.end(), archetypeName.begin(), ::tolower);
                    std::transform(filterText.begin(), filterText.end(), filterText.begin(), ::tolower);
                    if (archetypeName.find(filterText) != std::string::npos || filterText.empty()) {
                        archetypeFilteredList.emplace_back(itr->second);
                        i++;

                    }
                }
                archetypeFilterRequestQueue.pop();
                // printf("processed: %s\n", filterText.c_str());
            }
        }


        std::vector <std::string> actions{
	        "ActivateWander",
	        "AddCollisionObserver",
	        "AddFacingDesire",
	        "AddInboundModifier",
	        "AddLookDesire",
	        "AddMovementDesire",
	        "AddNoiseStateDesire",
	        "AddSpeedDesire",
	        "AttachPistol",
	        "BecomeActive",
	        "BecomeBroken",
	        "BecomeInactive",
	        "BeginAnimatedDistraction",
	        "BeginAnimatedStunned",
	        "BeginDamagedAudio",
	        "BeginDisabledAudio",
	        "BeginLurking",
	        "BeginSearchExamine",
	        "BeginUndamagedAudio",
	        "BreakGlooEntity",
	        "BreakNearbyGloo",
	        "CalcHasSpaceToMimicSideStep",
	        "CallForHelp",
	        "ClearAllAttention",
	        "ClearAttentionOnPlayer",
	        "ClearConversationFacingTarget",
	        "ClearConversationLookTarget",
	        "ClearFear",
	        "ClearPrecache",
	        "Corrupt",
	        "CorruptNpc",
	        "CorruptedMovementGlitch",
	        "CreateAiTree",
	        "DeactivateWander",
	        "DestroyAiTree",
	        "Dissipate",
	        "DoCorruptExplode",
	        "EndAnimatedDistraction",
	        "EndAnimatedStunned",
	        "EndDormant",
	        "EndLurking",
	        "EndMimicry",
	        "EndPsiSuppressed",
	        "EndSearchExamine",
	        "FearAOEOnPlayerKill",
	        "FindDeathInstigator",
	        "FindNearestReachablePoint",
	        "FindNearestValidPoint",
	        "FinishRaiseFromCorpse",
	        "FlowGraphDisableAi",
	        "FlowGraphIgnoreDistractions",
	        "FlowGraphSetDumbed",
	        "FlowgraphDisableCombatIntensity",
	        "FlowgraphEnableCombatIntensity",
	        "FollowLeaderEntity",
	        "ForceDesperationMimicryInPlace",
	        "ForceDownToSearchAttentionLevel",
	        "ForceMovementReplan",
	        "ForceReevaluateMovementAnimAction",
	        "ForceStopCharacterEffect",
	        "ForceStopDistraction",
	        "FullSerialize",
	        "LightOnFire",
	        "LoadScriptVariables",
	        "LockKnownAttentionOnTarget",
	        "LockMimicry",
	        "MakeConscious",
	        "MakePoltergeistAppear",
	        "MakePoltergeistDisapear",
	        "MimicAndReplaceEntity",
	        "MimicCollisionRebound",
	        "MimicEntity",
	        "NoticeTargetPosition",
	        "NotifyCorruptedOwnerOfUncorruption",
	        "OverrideCharacterId",
	        "OverrideHitReactAnimation",
	        "PerformAnimatedAbility",
	        "PerformCombatReaction",
	        "PerformFatality",
	        "PerformHitReactShift",
	        "PerformMimicGlitch",
	        "PerformMimicReorientation",
	        "PerformMindControlled",
	        "PerformPassiveGlooBreak",
	        "PerformPatrolIdle",
	        "PerformPostSerializeGloodPose",
	        "PerformReorientation",
	        "PerformScan",
	        "PerformStopMindControlled",
	        "PerformUnanimatedAbility",
	        "PerformWanderIdle",
	        "PlayMimicHitEffect",
	        "PlayMimicryMorphInEffect",
	        "PlayMimicryMorphOutEffect",
	        "Ragdoll",
	        "RefreshCombatIntensity",
	        "RefreshNotice",
	        "RegisterOnFleeNodeCallsbacks",
	        "Release",
	        "ReloadAmmo",
	        "RemoveAmmoAmount",
	        "RemoveCollisionObserver",
	        "RemoveFacingDesire",
	        "RemoveHead",
	        "RemoveInboundModifier",
	        "RemoveLookDesire",
	        "RemoveMovementDesire",
	        "RemoveNoiseStateDesire",
	        "RemoveSpeedDesire",
	        "RequestDistraction",
	        "RequestLurk",
	        "RequestStopLurk",
	        "ResetAIAlwaysUpdate",
	        "ResetPlayerBumpTimer",
	        "Resist",
	        "ScaleLookDesireSpeed",
	        "Serialize",
	        "ShiftBegin",
	        "ShiftEnd",
	        "ShiftTelegraph",
	        "ShouldBecomeRigidOnGloo",
	        "ShouldMimicRebound",
	        "SpawnLootOnDeath",
	        "StartAmbientSound",
	        "StartAttackGlow",
	        "StartAttackJitter",
	        "StartCharacterEffect",
	        "StartCowering",
	        "StartEnergizedDeath",
	        "StartGlooEffects",
	        "StartInstantDeath",
	        "StartMaterialAnimation",
	        "StartOnFireEffect",
	        "StartOperatorArmIdle",
	        "StartPsiSuppressed",
	        "StartRaiseFromCorpse",
	        "StartRaisePhantomCast",
	        "StartStun",
	        "StartXRayVisionOn",
	        "StopCharacterEffect",
	        "StopCowering",
	        "StopGlooEffects",
	        "StopMimicGlitch",
	        "StopOnFireEffect",
	        "StopOperatorArmIdle",
	        "StopStun",
	        "StopXRayVisionOn",
	        "SupportsLookAt",
	        "TestInteraction",
	        "TryDodge",
	        "TryEvaluateAbilityContext",
	        "TryEvaluateAndPerformAbilityContext",
	        "TryPerformAbilityContext1",
	        "TryPerformAbilityContext2",
	        "TryPerformAbilityContext3",
	        "TryPerformAnyAbility",
	        "UnCorrupt",
	        "UnlockMimicry",
	        "Update",
	        "UpdateFactions",
	        "UpdateMaterialAnimation",
	        "WaitForAnimatedStunned"
        };
        std::vector<std::string> is{
            "IsAlert",
            "IsAsleep",
            "IsAttentionProxyFollowing",
            "IsBreakable",
            "IsBroken",
            "IsCharacterEffectEnabled",
            "IsCorrupted",
            "IsCowering",
            "IsDead",
            "IsDestroyed",
            "IsDissipating",
            "IsDormant",
            "IsEntityMimicable",
            "IsEthericDoppelganger",
            "IsFalling",
            "IsFeared",
            "IsFollowingPlayer",
            "IsGlooSlow",
            "IsGoingToFreezeInGloo",
            "IsHacked",
            "IsIgnoringRangedAbilities",
            "IsInRaiseFromCorpseAnim",
            "IsJumping",
            "IsLureFlareValid",
            "IsLurking",
            "IsMimic",
            "IsMimicking",
            "IsMindControlled",
            "IsMovementAnimated",
            "IsMovementDesireExecuting",
            "IsMovementInterruptible",
            "IsNpcHealthUIDisabled",
            "IsPerformingAbility",
            "IsPlayerControlled",
            "IsPsiLifted",
            "IsPsiSuppressed",
            "IsRagdolled",
            "IsResisting",
            "IsScrunched",
            "IsShifting",
            "IsStandingUp",
            "IsStunned",
            "IsVisible",
            "IsVulnerable",
            "IsWithinCameraViewCone",
            "IsXRayTrackingTarget",
            "CanAttachGloo",
            "CanBeDistracted",
            "CanBeHypnotized",
            "CanDeathReact",
            "CanHitReact",
            "CanInstigate",
            "CanJump",
            "CanMimicSideStepLeft",
            "CanMimicSideStepRight",
            "CanPerformAbilityContext",
            "CanPerformAnimatedAbility",
            "CanPerformFatality",
            "CanPerformHitReactShift",
            "CanPerformUnanimatedAbility",
            "CanRaiseAnotherPhantom",
            "CanSee",
            "CanSpeak"
        };
        std::vector<std::string> get{
            "ActiveEthericDoppelgangerId",
            "AmmoCount",
            "AttachedPistolId",
            "Attachment",
            "AttentionOnPlayer",
            "CachedThrowAtLocation",
            "CachedThrowAtType",
            "CharacterEffectsManager",
            "ComplexHearingGain",
            "ComplexVisionGain",
            "CurrentAbilityContextId",
            "CurrentCombatRole",
            "DefaultFaction",
            "DesiredSpeed",
            "DodgeTimeStamp",
            "EnergizedAmount",
            "EntityPoolSignature",
            "EthericDoppelgangerOwnerId",
            "GlooEffect",
            "HeadDirection",
            "HeadPosition",
            "HitReactKnockDownTimeStamp",
            "HitReactStaggerTimeStamp",
            "IsHitReactingPrereqFlag",
            "IsRegainingLosPrereqFlag",
            "IsShuttingDownHack",
            "IsTakingDamagePrereqFlag",
            "LastPerformedContextId",
            "LeaderId",
            "MaterialAnimationDuration",
            "MaterialAnimationManager",
            "MimicingEntityId",
            "MimicryReason",
            "MovementRequestId",
            "NpcManagerCombatPoints",
            "NumControlledTurrets",
            "NumCorruptedNpcs",
            "PhysicalEntitiesInBox",
            "Properties",
            "RMIBase",
            "SearchReasonFromChangeReason",
            "TopAttentionTargetEntityId"
        };
        std::vector<std::string> set{
            "AnimTag",
            "BreakGlooEntity",
            "CachedThrowAtLocation",
            "ComplexHearingGain",
            "ComplexVisionGain",
            "ConversationFacingTarget",
            "ConversationLookTarget",
            "CurrentCombatRole",
            "EthericDoppelgangerId",
            "EthericDoppengangerOwnerId",
            "FearFromHostileDamage",
            "HasPerformedNotice",
            "HypnotizeStartTimeStamp",
            "IsEthericDoppelganger",
            "IsHitReactingPrereqFlag",
            "IsIgnoringRangedAbilities",
            "IsPlayerControlled",
            "IsRegainingLoSPrereqFlag",
            "PistolHidden",
            "ShouldSkipNextCombatReaction",
            "SpawnedState",
            "Stance",
            "TimeUntilDeath",
            "TransitionStateTag",
            "WanderAreaContainerEntity",
            "WanderFollowEntityId",
        };
        std::vector<std::string> statusPop{
             "AIAlwaysUpdate",
             "AttentionDrainLockOnTarget",
             "DisableAiTree",
             "DisableAllAmbientSounds",
             "DisableAttentionAndSenses",
             "DisableAttentionObjectAndPerceivables",
             "DisableAudible",
             "DisableBreakable",
             "DisableHearing",
             "DisableHitReactions",
             "DisableLifetimeEffect",
             "DisableNpcHealthUI",
             "DisableOperatorLevitatorsEffect",
             "DisableSenses",
             "DisableVisible",
             "DisableVision",
             "Dumbed",
             "EnableAbilities",
             "EnableAiTree",
             "EnableAttentionObject",
             "EnableAttentiveSubject",
             "EnableAudible",
             "EnableGlassBreaking",
             "EnableHearing",
             "EnableLifetimeEffect",
             "EnableOperatorLevitatorsEffect",
             "EnableRoomPerceiver",
             "EnableVisible",
             "EnableVision",
             "EnabledAmbientSound",
             "Fear",
             "ForceRigidOnGloo",
             "IndefiniteRagdoll"
        };
        std::vector<std::string> statusPush{
             "AIAlwaysUpdate",
             "AIAlwaysUpdateForPatrol",
             "AttentionDrainLockOnTarget",
             "DisableAiTree",
             "DisableAllAmbientSounds",
             "DisableAttentionAndSenses",
             "DisableAttentionObjectAndPerceivables",
             "DisableAudible",
             "DisableBreakable",
             "DisableDeathReactions",
             "DisableHearing",
             "DisableHitReactions",
             "DisableLifetimeEffect",
             "DisableNpcHealthUI",
             "DisableOperatorLevitatorsEffect",
             "DisableSenses",
             "DisableVisible",
             "DisableVision",
             "Dumbed",
             "EnableAbilities",
             "EnableAiTree",
             "EnableAttentionObject",
             "EnableAttentiveSubject",
             "EnableAudible",
             "EnableGlassBreaking",
             "EnableHearing",
             "EnableLifetimeEffect",
             "EnableOperatorLevitatorsEffect",
             "EnableRoomPerceiver",
             "EnableVisible",
             "EnableVision",
             "EnabledAmbientSound",
             "Fear",
             "ForceRigidOnGloo",
             "IndefiniteRagdoll"
        };
    };

    class chairloaderGuiPlayerManager {
    public:
        struct abilityEntry {
        public:
            uint64_t id;
            std::string name;
            bool acquired{ false };
        };
        static bool AbilityEntrySortByName(const abilityEntry& lhs, const abilityEntry& rhs) {
            return lhs.name < rhs.name;
        }
    private:
        const std::string modName = "ChrldrGUI.PlayerManager";
        bool AbilityListInitialized, refreshAbilityList;
        std::vector<abilityEntry> abilityDisplayList;
        std::queue<uint64_t> abilityRequestQueue;
    public:
        void draw(bool* bShow) {
            if (!ImGui::Begin("Player Manager", bShow)) {
                ImGui::End();
                return;
            }
            if (ImGui::BeginTabBar("Player Bar")) {
                if (ImGui::BeginTabItem("Inventories")) {

                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Abilities")) {
                    refreshAbilityList = ImGui::Button("Refresh");
                    if (ImGui::BeginChild("AbilityList", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_HorizontalScrollbar)) {
                        for (auto itr = abilityDisplayList.begin(); itr != abilityDisplayList.end(); ++itr) {
                            if (!itr->acquired) {
                                if (ImGui::Selectable(itr->name.c_str())) {
                                    abilityRequestQueue.push(itr->id);
                                }
                            }
                            else {
                                ImGui::TextColored(ImVec4{ 1.0f,1.0f,1.0f,0.7f }, "%s - acquired", itr->name.c_str());
                            }
                        }
                        ImGui::EndChild();
                    }
                    //TODO: figure out how to remove abilities too
                    // std::vector<ArkAbilityData>* abilities = &chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get()->m_abilities;
                    // if (!abilities->empty()) {
                    //     ImGui::Text("Size: %d\n", abilities->size());
                    //     int clip = 0;
                    //     for (auto itr = abilities->begin(); itr != abilities->end() && clip <= 1000; ++itr, clip++) {
                    //         if (itr->m_id != 0) {
                    //             static ArkAbilityLibrary library;
                    //             auto entry = library.arkAbilityMap.find(itr->m_id);
                    //             if(entry != library.arkAbilityMap.end()) {
                    //                 ImGui::Text(entry->second.c_str());
                    //             } else {
                    //                 ImGui::Text(std::to_string(itr->m_id).c_str());
                    //             }
                    //             
                    //         }
                    //     }
                    // }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::End();
        }
        void drawMenuBar(chairloaderGuiControl* control) {
                if (ImGui::BeginMenu("Player")) {
                    ImGui::MenuItem("Refresh Abilities");
                    ImGui::EndMenu();
                    // }
                }
        }

        // Handlers
        void checkAbilities(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            if (refreshAbilityList) {
                for (auto itr = abilityDisplayList.begin(); itr != abilityDisplayList.end(); ++itr) {
                    if (chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get() != nullptr) {
                        itr->acquired = chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get(), itr->id);
                    }
                    //abilityDisplayList.emplace_back(entry);
                }
                std::sort(abilityDisplayList.begin(), abilityDisplayList.end(), AbilityEntrySortByName);
                refreshAbilityList = false;
                log->logItem("Ability list refreshed", modName);
                // return logMessage{"Ability list refreshed", time(nullptr), logLevel::normal};
            }
        }
        void abilityRequestHandler(ChairloaderUtils* chairloader, chairloaderGuiLog* log) {
            try {
                if (!AbilityListInitialized) {
                    for (auto itr = chairloader->abilityLibrary.arkAbilityMap.begin(); itr != chairloader->abilityLibrary.arkAbilityMap.end(); ++itr) {
                        abilityEntry entry = { itr->first, itr->second, false };
                        if (chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get() != nullptr) {
                            if (chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get(), itr->first)) {
                                entry.acquired = true;
                            }
                        }
                        abilityDisplayList.emplace_back(entry);

                    }
                    std::sort(abilityDisplayList.begin(), abilityDisplayList.end(), AbilityEntrySortByName);
                    AbilityListInitialized = true;
                    log->logItem("Ability List Initialized", modName);
                    // return logMessage{"Ability List Initialized", time(nullptr), logLevel::normal};
                }
                else {
                    if (!abilityRequestQueue.empty()) {
                        uint64_t id = abilityRequestQueue.front();
                        abilityRequestQueue.pop();
                        abilityEntry* entry = nullptr;
                        for (auto itr = abilityDisplayList.begin(); itr != abilityDisplayList.end(); ++itr) {
                            if (itr->id == id) {
                                entry = itr._Ptr;
                            }
                        }
                        if (entry != nullptr) {
                            if (!entry->acquired) {
                                if (chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get() != nullptr && !chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get(), entry->id)) {
                                    chairloader->internalPreyFunctions->ArkAbilityComponentF->GrantAbility(chairloader->ArkPlayerPtr->m_playerComponent.m_pAbilityComponent.get(), entry->id);

                                    // printf("Granted Ability: %s\n", chairloader->abilityLibrary.arkAbilityMap.find(entry->id)->second.c_str());
                                    entry->acquired = true;
                                    std::string msg = "Granted Ability: " + chairloader->abilityLibrary.arkAbilityMap.find(entry->id)->second + "\n";
                                }
                                else {
                                    throw("Error, Ability Component Not Found");
                                }
                            }
                        }
                        else {
                            throw("Error, invalid ability id");
                        }
                    } 
                }
            }
            catch (char* c) {
                log->logItem(c, modName, logLevel::error);
                // return {c, time(nullptr), logLevel::error};
            }
        }
    };

    struct PreyConsole
    {
        char                  InputBuf[256];
        //TODO: implement a proper command structure
        ImVector<char*>       Items;
        ImVector<const char*> Commands;
        ImVector<char*>       History;
        int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
        ImGuiTextFilter       Filter;
        bool                  AutoScroll;
        bool                  ScrollToBottom;

        PreyConsole()
        {
            // IMGUI_DEMO_MARKER("Examples/Console");
            ClearLog();
            memset(InputBuf, 0, sizeof(InputBuf));
            HistoryPos = -1;

            // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
            // TODO: load prey console commands
            Commands.push_back("HELP");
            Commands.push_back("HISTORY");
            Commands.push_back("CLEAR");
            Commands.push_back("CLASSIFY");
            AutoScroll = true;
            ScrollToBottom = false;
            AddLog("Chairloader Console Initializeed");
        }
        ~PreyConsole()
        {
            ClearLog();
            for (int i = 0; i < History.Size; i++)
                free(History[i]);
        }

        // Portable helpers
        static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
        static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
        static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
        static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

        void    ClearLog()
        {
            for (int i = 0; i < Items.Size; i++)
                free(Items[i]);
            Items.clear();
        }

        void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
            // FIXME-OPT
            char buf[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
            buf[IM_ARRAYSIZE(buf) - 1] = 0;
            va_end(args);
            Items.push_back(Strdup(buf));
        }

        void Draw(bool* p_open)
        {
            // ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
            if (!ImGui::BeginChild("Console", ImGui::GetContentRegionAvail(), true, ImGuiWindowFlags_None)) {
                ImGui::EndChild();
                return;
			}
            // if (!ImGui::Begin("Console", p_open)) {
            //     ImGui::End();
            //     return;
            // }

            // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
            // So e.g. IsItemHovered() will return true when hovering the title bar.
            // Here we create a context menu only available from the title bar.
            // if (ImGui::BeginPopupContextItem())
            // {
            //     if (ImGui::MenuItem("Close Console"))
            //         *p_open = false;
            //     ImGui::EndPopup();
            // }

            ImGui::TextWrapped(
                "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
                "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
            ImGui::TextWrapped("Enter 'HELP' for help.");

            // TODO: display items starting from the bottom

            if (ImGui::SmallButton("Add Debug Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear")) { ClearLog(); }
            ImGui::SameLine();
            bool copy_to_clipboard = ImGui::SmallButton("Copy");
            //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

            ImGui::Separator();

            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Options, Filter
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
            ImGui::Separator();

            // Reserve enough left-over height for 1 separator + 1 input text
            const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) ClearLog();
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                ImGui::LogToClipboard();
            for (int i = 0; i < Items.Size; i++)
            {
                const char* item = Items[i];
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(item);
                if (has_color)
                    ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            ImGui::PopStyleVar();
            ImGui::EndChild();
            ImGui::Separator();

            // Command-line
            bool reclaim_focus = false;
            ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
            if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
            {
                char* s = InputBuf;
                Strtrim(s);
                if (s[0])
                    ExecCommand(s);
                strcpy_s(s, 0, "");
                reclaim_focus = true;
            }

            // Auto-focus on window apparition
            ImGui::SetItemDefaultFocus();
            if (reclaim_focus)
                ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

            ImGui::EndChild();
        }

        void    ExecCommand(const char* command_line)
        {
            AddLog("# %s\n", command_line);

            // Insert into history. First find match and delete it so it can be pushed to the back.
            // This isn't trying to be smart or optimal.
            HistoryPos = -1;
            for (int i = History.Size - 1; i >= 0; i--)
                if (Stricmp(History[i], command_line) == 0)
                {
                    free(History[i]);
                    History.erase(History.begin() + i);
                    break;
                }
            History.push_back(Strdup(command_line));

            // Process command
            //TODO: Implement prey command execution and writeback
            if (Stricmp(command_line, "CLEAR") == 0)
            {
                ClearLog();
            }
            else if (Stricmp(command_line, "HELP") == 0)
            {
                AddLog("Commands:");
                for (int i = 0; i < Commands.Size; i++)
                    AddLog("- %s", Commands[i]);
            }
            else if (Stricmp(command_line, "HISTORY") == 0)
            {
                int first = History.Size - 10;
                for (int i = first > 0 ? first : 0; i < History.Size; i++)
                    AddLog("%3d: %s\n", i, History[i]);
            }
            else
            {
                AddLog("Unknown command: '%s'\n", command_line);
            }

            // On command input, we scroll to bottom even if AutoScroll==false
            ScrollToBottom = true;
        }

        // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
        static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
        {
            PreyConsole* console = (PreyConsole*)data->UserData;
            return console->TextEditCallback(data);
        }

        int     TextEditCallback(ImGuiInputTextCallbackData* data)
        {
            //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
            switch (data->EventFlag)
            {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
            case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
            }
            return 0;
        }
    };
    chairloaderGuiControl control;
    chairloaderGuiLog log;
    chairloaderGuiPlayerManager playerManager; 
    chairloaderGuiEntityManager entityManager{chairloaderGlobal};
    PreyConsole console;

    // std::vector<std::string> modsWithDrawFuncs;
    std::vector<std::tuple<std::function<void()>, std::string>> drawFuncs;
    std::mutex drawHandleMutex;
public:
    
    void logItem(std::string msg, const std::string modName, logLevel level = logLevel::normal, bool displayToScreen = true) {
        log.logItem(msg, modName, level, displayToScreen);
    }
    void logItem(logMessage message, bool displayToScreen = true) {
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
                        log.logItem("Welcome to funland sonic", modName, logLevel::error);
                    }
                    ImGui::EndMenu();
                }
                playerManager.drawMenuBar(&control);
                entityManager.drawMenuBar(&control, chairloaderGlobal);
                // if (control.showEntityManager) {
                    
                // if (control.showPlayerManager) {
                    
                // if (control.showConsole) {
                    
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
    void Handlers() {
        
        if (!chairloaderGlobal->preyEnvironmentPointers->pGame->m_pFramework->IsInLevelLoad() || !chairloaderGlobal->preyEnvironmentPointers->pGame->m_pFramework->IsLoadingSaveGame()) {
            drawHandleMutex.lock();
        	entityManager.archetypeFilterRequestHandler(chairloaderGlobal, &log);
            entityManager.filterEntityList(chairloaderGlobal, &log);

            playerManager.checkAbilities(chairloaderGlobal, &log);
            drawHandleMutex.unlock();
            if (!chairloaderGlobal->preyEnvironmentPointers->pSystem->IsPaused()) {
                entityManager.archetypeSpawnRequestHandler(chairloaderGlobal, &log);
                entityManager.entityModifyHandler(chairloaderGlobal, &log);

                playerManager.abilityRequestHandler(chairloaderGlobal, &log);
            }
        }
        
    }
    ChairloaderGui(ChairloaderUtils* chairloaderIn) {
        chairloaderGlobal = chairloaderIn;
    }
};

