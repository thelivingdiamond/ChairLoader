#pragma once
#include "pch.h"
namespace ImGuiUtils {
    inline void RightAlignText(const char* text) {
        auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text).x
            - ImGui::GetScrollX());
        if (posX > ImGui::GetCursorPosX())
            ImGui::SetCursorPosX(posX);
    }
    inline void HelpMarker(const char* desc) {
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
};