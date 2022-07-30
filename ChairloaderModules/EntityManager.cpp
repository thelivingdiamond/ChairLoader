#include "pch.h"
#include "EntityManager.h"

#include "Logging.h"
#include "Prey/Cry3DEngine/I3DEngine.h"
#include "Prey/CryEntitySystem/EntityArchetype.h"
#include "Prey/CryEntitySystem/EntitySystem.h"
#include "Prey/CryEntitySystem/Entity.h"
#include "Prey/GameDll/ark/player/ArkPlayer.h"


EntityManager::EntityManager(ChairloaderGlobalEnvironment* env) {
    gCLEnv = env;
    oldArchetypeFilterText = " ";
}

EntityManager::~EntityManager() {

}

void EntityManager::drawEntitySpawner(bool* bShow) {
    if (ImGui::Begin("Entity Spawner", bShow)) {
        ImGui::TextColored(color, statusMessage.c_str());

        if (archetypeToSpawn == nullptr) {
            ImGui::Text("Entity Archetype Name: ");
            ImGui::Text("Entity ID: ");
        }
        else {
            ImGui::Text("Entity Archetype Name: %s", archetypeToSpawn->GetName());
            ImGui::Text("Entity ID: %d", archetypeToSpawn->GetId());
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
            spawnEntity();
        }
        ImGui::Text("Filter:");
        static bool initializedList = false;
        if(ImGui::InputText("##filter text", &archetypeFilterText) || !initializedList){
            oldArchetypeFilterText = archetypeFilterText;
            initializedList = true;
            auto &archetypeNameList = ((CEntitySystem*)gEnv->pEntitySystem)->m_pEntityArchetypeManager->m_nameToArchetypeMap;
            auto itr = archetypeNameList.begin();
            archetypeFilteredList.clear();
            for (int i = 0; i < 400 && itr != archetypeNameList.end(); ++itr) {
                std::string archetypeName = itr->second->GetName();
                // size_t last_period = archetypeName.find_last_of('.');
                // archetypeName = archetypeName.substr(last_period, archetypeName.size());
                std::transform(archetypeName.begin(), archetypeName.end(), archetypeName.begin(), ::tolower);
                std::transform(archetypeFilterText.begin(), archetypeFilterText.end(), archetypeFilterText.begin(), ::tolower);
                if (archetypeName.find(archetypeFilterText) != std::string::npos || archetypeFilterText.empty()) {
                    archetypeFilteredList.emplace_back(itr->second);
                    i++;
            
                }
            }
        }
        ImGuiUtils::HelpMarker("Filter usage:\n"
            "  \"\"         display all lines\n"
            "  \"xxx\"      display lines containing \"xxx\"\n"
            "  \"xxx,yyy\"  display lines containing \"xxx\" or \"yyy\"\n"
            "  \"-xxx\"     hide lines containing \"xxx\"");

        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable;
        ImVec2 outer_size = ImVec2(0.0f, 0.0f);

        if (ImGui::BeginTable("EntityArchetypeList", 2, flags, outer_size)) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None);
            ImGui::TableHeadersRow();

            auto itr = archetypeFilteredList.begin();
            for (int clip = 0; clip < 200 && itr != archetypeFilteredList.end(); clip++, ++itr) {
                IEntityArchetype* archetype = *itr;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                if(ImGui::Selectable(archetype->GetName(), archetypeToSpawn == *itr, ImGuiSelectableFlags_SpanAllColumns)) {
                    archetypeToSpawn = archetype;
                }

                // if (ImGui::BeginPopupContextItem(NULL, ImGuiPopupFlags_MouseButtonLeft)) // <-- use last item id as popup id
                // {
                //     ImGui::Text("Display information about the archetype here");
                //     if (ImGui::Button("Choose"))
                //         archetypeToSpawn = archetype;
                //     if (ImGui::Button("Close"))
                //         ImGui::CloseCurrentPopup();
                //     ImGui::EndPopup();
                // }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%llu", archetype->GetId());
            }

            if (time(NULL) > statusTimer + 3)
                statusMessage = "";
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void EntityManager::drawEntityList(bool* bShow) {
    if (ImGui::Begin("Entity List", bShow)) {
        {
            ImGui::BeginChild("left pane", ImVec2(250, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::InputText("##input", &filterText);
            if (oldFilterText != filterText) {
                oldFilterText = filterText;
                refreshDisplayList = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("Refresh")) {
                refreshDisplayList = true;
            }
            if(refreshDisplayList) {
                entityDisplayList.clear();
                selected = nullptr;
                for (auto itr = GetEntitySystem()->m_EntityArray.begin(); itr != GetEntitySystem()->m_EntityArray.end(); ++itr) {
                    if (*itr != nullptr) {
                        if ((*itr)->m_szName != nullptr) {
                            std::string name = (*itr)->m_szName.c_str();
                            std::string newFilterText = filterText;
                            std::transform(newFilterText.begin(), newFilterText.end(), newFilterText.begin(), ::tolower);
                            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                            if (name.find(newFilterText) != name.npos || filterText.empty()) {
                                entityDisplayList.emplace_back(*itr);
                            }
                        }
                    }
                }
            }
            auto itr = entityDisplayList.begin();
            for (int i = 0; itr != entityDisplayList.end() && i < 500; i++, ++itr) {
                std::string label;
                if ((*itr)->m_szName.c_str() != nullptr) {
                    label = (*itr)->m_szName.c_str();
                }
                else {
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
            }
            else {
                ImGui::Text("Entity: %s", "Null");
            }
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton))
            {
                if (ImGui::BeginTabItem("Entity Details"))
                {
                    // CryLog("at the entity check\n");

                    if (selected != nullptr) {
                        if (ImGui::BeginTable("Details", 2, ImGuiTableFlags_NoClip | ImGuiTableFlags_BordersH)) {
                            // Setup Columns
                            ImGui::TableSetupColumn(" Item:", ImGuiTableColumnFlags_WidthFixed, 75.0f);
                            ImGui::TableSetupColumn(" Value:");
                            // Name
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGuiUtils::RightAlignText("Name:");
                            ImGui::Text("Name:");
                            ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                            ImGui::TableNextColumn();
                            if (selected->m_szName.c_str() != nullptr)
                                ImGui::Text("%s", selected->m_szName.c_str());
                            else
                                ImGui::Text("%s", "--Null Name--");
                            // ID
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGuiUtils::RightAlignText("ID:");
                            ImGui::Text("ID:");
                            ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                            ImGui::TableNextColumn();
                            // CryLog("at id\n");
                            ImGui::Text("%llu", selected->m_nID);
                            // GUID
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            // CryLog("at guid\n");
                            ImGuiUtils::RightAlignText("GUID:");
                            ImGui::Text("GUID:");
                            ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                            ImGui::TableNextColumn();
                            ImGui::Text("%llu", selected->m_guid);
                            // Class
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            // CryLog("at class\n");
                            ImGuiUtils::RightAlignText("Class:");
                            ImGui::Text("Class:");
                            ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                            ImGui::TableNextColumn();
                            if (selected->m_pClass != nullptr) {
                                if (selected->m_pClass->GetName() != nullptr)
                                    ImGui::Text("%s", selected->m_pClass->GetName());
                                else
                                    ImGui::Text("%s", "--Unknown Class--");
                            }
                            else {
                                ImGui::Text("%s", "--Null Class--");
                            }
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGuiUtils::RightAlignText("Archetype:");
                            ImGui::Text("Archetype:");
                            ImGui::SetCursorPosX(ImGui::GetColumnWidth());
                            ImGui::TableNextColumn();
                            if (selected->m_pArchetype != nullptr) {
                                if (selected->m_pArchetype->GetName() != nullptr)
                                    ImGui::Text("%s", selected->m_pArchetype->GetName());
                                else
                                    ImGui::Text("%s", "--Unknown Archetype--");
                            }
                            else {
                                ImGui::Text("%s", "--Null Archetype--");
                            }

                            ImGui::EndTable();
                        }
                        static float scale;
                        ImGui::Text("Position: ");
                        ImGui::InputFloat("x", &selected->m_vPos.x, 0, 0, "% .1f");
                        ImGui::SameLine();
                        ImGui::InputFloat("y", &selected->m_vPos.y, 0, 0, "% .1f");
                        ImGui::SameLine();
                        ImGui::InputFloat("z", &selected->m_vPos.z, 0, 0, "% .1f");
                        if(ImGui::Button("Set to player pos")) {
                            ((IEntity*)selected)->SetPos(gCLEnv->entUtils->ArkPlayerPtr()->GetEntity()->GetPos());
                        }


                        ImGui::Text("Rotation: ");
                        ImGui::InputFloat("x", &selected->m_qRotation.v.x, 0, 0, "% .1f");
                        ImGui::SameLine();
                        ImGui::InputFloat("y", &selected->m_qRotation.v.y, 0, 0, "% .1f");
                        ImGui::SameLine();
                        ImGui::InputFloat("z", &selected->m_qRotation.v.z, 0, 0, "% .1f");
                        ImGui::SameLine();
                        ImGui::InputFloat("w", &selected->m_qRotation.w, 0, 0, "% .1f");

                        ImGui::Text("Scale: ");
                        if(ImGui::InputFloat("scale", &scale, 0, 0, "% .1f")) {
                            Vec3 newScale{ scale, scale, scale };
                            ((IEntity*)selected)->SetScale(newScale);
                        }
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
                        ImGui::Button("Push");
                    }
                    if (std::find(statusPop.begin(), statusPop.end(), selectedStatus) != statusPop.end()) {
                        ImGui::SameLine();
                        ImGui::Button("Pop");
                    }
                    ImGui::EndTabItem();
                }
                // TODO: ALL OF THIS
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
                                ImGui::InputText("##NPC", &text, ImGuiInputTextFlags_ReadOnly);
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
                // ENDTODO
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            // if (ImGui::Button("Revert")) {}
            // ImGui::SameLine();
            // if (ImGui::Button("Save")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

void EntityManager::drawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Entity")) {
            if (ImGui::BeginMenu("Spawn Entity")) {
                ImGui::MenuItem("Spawn Last Spawned", nullptr, false, archetypeToSpawn != nullptr);
                ImGui::Separator();
                ImGui::TextDisabled("Quick Spawns");
                if (ImGui::BeginMenu("Spawn Typhon")) {
                    if (ImGui::BeginMenu("Mimics")) {
                        if (ImGui::MenuItem("Mimic"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Mimics.Mimic);
                        if (ImGui::MenuItem("Greater Mimic"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Mimics.EliteMimic);
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Phantoms")) {
                        if (ImGui::MenuItem("Normal"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Phantoms.BasePhantom);
                        if (ImGui::MenuItem("Thermal"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Phantoms.ThermalPhantom);
                        if (ImGui::MenuItem("Etheric"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Phantoms.EthericPhantom);
                        if (ImGui::MenuItem("Voltaic"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Phantoms.VoltaicPhantom);
                        ImGui::EndMenu();
                    }
                    if (ImGui::MenuItem("Nightmare"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.ArkNightmare);
                    if (ImGui::MenuItem("Poltergeist"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.ArkPoltergeist);
                    if (ImGui::MenuItem("Telepath"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Overseers.Telepath);
                    if (ImGui::MenuItem("Technopath"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkNpcs.Overseers.Technopath);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Spawn Operator")) {
                    if (ImGui::MenuItem("Medical"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkRobots.Operators.Generic.MedicalOperator);
                    if (ImGui::MenuItem("Engineering"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkRobots.Operators.Generic.EngineeringOperator);
                    if (ImGui::MenuItem("Science"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkRobots.Operators.Generic.ScienceOperator);
                    if (ImGui::MenuItem("Military"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkRobots.Operators.Generic.MilitaryOperator);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Spawn Turrets")) {
                    if (ImGui::MenuItem("Turret"))
                        quickSpawnEntity(entityArchetypeLibrary.ArkRobots.Turrets.Turret_Default);
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Spawn Items")) {
                    if (ImGui::BeginMenu("Weapons")) {
                        if (ImGui::MenuItem("Wrench"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.Wrench);
                        if (ImGui::MenuItem("Shotgun"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.Shotgun);
                        if (ImGui::MenuItem("Shotgun Ammo"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Ammo.ShotgunShells);
                        if (ImGui::MenuItem("Pistol"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.Pistol);
                        if (ImGui::MenuItem("Pistol Ammo"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Ammo.PistolBullets);
                        if (ImGui::MenuItem("Gloo Gun"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.GooGun);
                        if (ImGui::MenuItem("Gloo Gun Ammo"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Ammo.GooGun);
                        if (ImGui::MenuItem("Q Beam"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.Instalaser);
                        if (ImGui::MenuItem("Q Beam Ammo"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Ammo.InstaLaser);
                        if (ImGui::MenuItem("Stun Gun"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Weapons.StunGun);
                        if (ImGui::MenuItem("Stun Gun Ammo"))
                            quickSpawnEntity(entityArchetypeLibrary.ArkPickups.Ammo.StunGunAmmo);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Toggle All")) {
                if (showEntityList && showEntitySpawner) {
                    showEntityList = false;
                    showEntitySpawner = false;
                }
                else {
                    showEntityList = true;
                    showEntitySpawner = true;
                }
            }
            ImGui::MenuItem("Show Entity List", nullptr, &showEntityList);
            ImGui::MenuItem("Show Entity Spawner", nullptr, &showEntitySpawner);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void EntityManager::spawnEntity() {
    try {
        if (archetypeToSpawn != nullptr) {
            static Vec3 pos;
            static Quat rot = Quat{ 0,0,0,1 };
            archetypeToSpawn;
            usePlayerPos;
            offsetFromPlayer;
            if (usePlayerPos) {
                pos = gEntUtils->ArkPlayerPtr()->GetEntity()->GetPos();
                if (offsetFromPlayer) {
                    pos.x += gEntUtils->ArkPlayerPtr()->m_cachedReticleDir.x * 5;
                    pos.y += gEntUtils->ArkPlayerPtr()->m_cachedReticleDir.y * 5;
                }
            }
            else {
                pos = Vec3{ spawnX, spawnY, spawnZ };
            }
            std::string archetypeName = archetypeToSpawn->GetName();
            // if an npc use npc spawning
            if ((archetypeName.find("ArkRobots") != std::string::npos || archetypeName.find("ArkHumans") != std::string::npos || archetypeName.find("ArkNpcs") != std::string::npos) && archetypeName != "Turrets.Turret_Default") {
                gEntUtils->spawnNpcFromArchetype(inputName.c_str(), pos, rot, archetypeToSpawn->GetId(), spawnCount);
            }
            else {
                pos.z += 0.2f;
                gEntUtils->spawnEntityFromArchetype(inputName.c_str(), pos, rot, archetypeToSpawn->GetId(), spawnCount);
            }
            // done

            color = { 1,1,1,1 };
            statusMessage = "spawned an entity: " + inputName;
            time(&statusTimer);
        }
        else {
            throw("Error, no archetype selected");
        }
    }
    catch (std::string& c) {
        color = { 1,0,0,1 };
        statusMessage = c;
        time(&statusTimer);
    }
}

void EntityManager::quickSpawnEntity(uint64_t archetypeId) {
    try {
        if (gEnv->pEntitySystem->GetEntityArchetype(archetypeId) != nullptr) {
            static Vec3 pos;
            static Quat rot = Quat{ 0,0,0,1 };
            pos = ArkPlayer::GetInstancePtr()->GetEntity()->GetPos();
            pos.x += ArkPlayer::GetInstancePtr()->m_cachedReticleDir.x * 5;
            pos.y += ArkPlayer::GetInstancePtr()->m_cachedReticleDir.y * 5;
            std::string archetypeName = gEnv->pEntitySystem->GetEntityArchetype(archetypeId)->GetName();

            if ((archetypeName.find("ArkRobots") != std::string::npos || archetypeName.find("ArkHumans") != std::string::npos || archetypeName.find("ArkNpcs") != std::string::npos) && archetypeName != "Turrets.Turret_Default") {
                gCLEnv->entUtils->spawnNpcFromArchetype("", pos, rot, archetypeId, 1);
            }
            else {
                gCLEnv->entUtils->spawnEntityFromArchetype("", pos, rot, archetypeId, 1);
            }
            gCLEnv->gui->logItem("spawned an entity: " + inputName, moduleName);
        }
        else {
            throw("Error, no archetype found");
        }
    }
    catch (std::string& c) {
        gCLEnv->gui->logItem(c, moduleName, logLevel::error);
    }
}



void EntityManager::Draw(ImGuiContext* gContext) {
    ImGui::SetCurrentContext(gContext);
    drawMenuBar();
    drawEntityList(&showEntityList);
    drawEntitySpawner(&showEntitySpawner);
}

void EntityManager::InitSystem() {}
void EntityManager::InitGame(IGameFramework* pFramework) {}
void EntityManager::PreUpdate() {}
void EntityManager::PostUpdate() {}
void EntityManager::ShutdownGame() {}
void EntityManager::ShutdownSystem() {}