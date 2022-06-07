#pragma once
#include "pch.h"

#include "ArkBasicTypes.h"
#include "ChairloaderUtils.h"
#include "preyDllObjects.h"
#include "ArkEntityArchetypeLibrary.h"
#include "GUIUtils.h"
class ChairloaderGUIPlayerManager
{
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
    const std::string modName = "ChairloaderGUI.PlayerManager";
    bool AbilityListInitialized, refreshAbilityList;
    std::vector<abilityEntry> abilityDisplayList;
    std::queue<uint64_t> abilityRequestQueue;
    // TODO: inventory items
    std::map<uint32_t, StorageCell> inventoryItems;
public:
    ChairloaderGUIPlayerManager() = default;

    void draw(bool* bShow) {
        if (!ImGui::Begin("Player Manager", bShow)) {
            ImGui::End();
            return;
        }
        if (ImGui::BeginTabBar("Player Bar")) {
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
                // std::vector<ArkAbilityData>* abilities = &chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get()->m_abilities;
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
            if (ImGui::BeginTabItem("Inventories")) {
                if (ImGui::BeginChild("InventoryTable", ImVec2(0, 0), true)) {
                    if (ImGui::BeginTable("InventoryTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH)) {
                        ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("x", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("y", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableHeadersRow();
                        for (auto& item : inventoryItems) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%d", item.first);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%d", item.second.m_x);
                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%d", item.second.m_y);
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndChild();
                }


                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
    void drawMenuBar(bool* control) {
        if (ImGui::BeginMenu("Player")) {
            ImGui::MenuItem("Refresh Abilities");
            ImGui::EndMenu();
            // }
        }
    }

    void update(ChairloaderUtils* chairloader, ChairloaderGUILog* log) {
        if (!chairloader->preyEnvironmentPointers->pSystem->IsPaused()) {
            abilityRequestHandler(chairloader, log);
        }
        checkAbilities(chairloader, log);
    }
private:
    // Handlers
    void checkAbilities(ChairloaderUtils* chairloader, ChairloaderGUILog* log) {
        if (refreshAbilityList) {
            if (chairloader->ArkPlayerPtr() != nullptr) {
                for (auto itr = abilityDisplayList.begin(); itr != abilityDisplayList.end(); ++itr) {
                    if (chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get() != nullptr) {
                        itr->acquired = chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get(), itr->id);
                    }
                    //abilityDisplayList.emplace_back(entry);
                }
                std::sort(abilityDisplayList.begin(), abilityDisplayList.end(), AbilityEntrySortByName);
                refreshAbilityList = false;
                log->logItem("Ability list refreshed", modName);
                // return logMessage{"Ability list refreshed", time(nullptr), logLevel::normal};
            }
            else {
                log->logItem("Player pointer null", modName);
            }
        }
    }
    void abilityRequestHandler(ChairloaderUtils* chairloader, ChairloaderGUILog* log) {
        try {
            if (!AbilityListInitialized) {
                if (chairloader->ArkPlayerPtr() != nullptr) {
                    for (auto itr = chairloader->abilityLibrary.arkAbilityMap.begin(); itr != chairloader->abilityLibrary.arkAbilityMap.end(); ++itr) {
                        abilityEntry entry = { itr->first, itr->second, false };
                        if (chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get() != nullptr) {
                            if (chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get(), itr->first)) {
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
                    throw("Player pointer null");
                }
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
                            if (chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get() != nullptr && !chairloader->internalPreyFunctions->ArkAbilityComponentF->HasAbility(chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get(), entry->id)) {
                                chairloader->internalPreyFunctions->ArkAbilityComponentF->GrantAbility(chairloader->ArkPlayerPtr()->m_playerComponent.m_pAbilityComponent.get(), entry->id);

                                // printf("Granted Ability: %s\n", chairloader->abilityLibrary.arkAbilityMap.find(entry->id)->second.c_str());
                                entry->acquired = true;
                                // std::string msg = "Granted Ability: " + chairloader->abilityLibrary.arkAbilityMap.find(entry->id)->second + "\n";
                                log->logItem(("Granted Ability: " + chairloader->abilityLibrary.arkAbilityMap.find(entry->id)->second + "\n"), modName);
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
        catch (std::string c) {
            log->logItem(c, modName, ChairloaderGUILog::logLevel::error);
        }
    }
};

