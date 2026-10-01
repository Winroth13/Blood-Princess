#include "statistics_panel.hpp"
#include "core/debug.hpp"

#include "imgui/imgui.h"

void StatisticsPanel::ImGuiRender() {
    ImGui::SetNextWindowSize({-FLT_MIN, -FLT_MIN}, ImGuiCond_Appearing);
    if (!ImGui::Begin("Statistics")) {
        ImGui::End();
        return;
    }

    for (const auto &[name, value] : Debug::GetStatistic())
        ImGui::Text("%s: %s", name.c_str(), value.c_str());

    ImGui::End();
}