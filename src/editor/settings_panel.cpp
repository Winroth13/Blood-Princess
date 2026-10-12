#include "settings_panel.hpp"
#include "core/debug.hpp"

#include "imgui/imgui.h"

void SettingsPanel::ImGuiRender() {
    ImGui::SetNextWindowSize({-FLT_MIN, -FLT_MIN}, ImGuiCond_Appearing);
    if (!ImGui::Begin("Settings")) {
        ImGui::End();
        return;
    }

    for (const auto &[name, value] : Debug::GetSettings()) {
        bool newValue = value;
        if (ImGui::Checkbox(name.c_str(), &newValue))
            Debug::SetSetting(name, newValue);
    }

    ImGui::End();
}