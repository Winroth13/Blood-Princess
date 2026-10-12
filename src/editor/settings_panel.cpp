#include "settings_panel.hpp"
#include "core/debug.hpp"

#include "imgui/imgui.h"

void SettingsPanel::ImGuiRender() {
    ImGui::SetNextWindowSize({-FLT_MIN, -FLT_MIN}, ImGuiCond_Appearing);
    if (!ImGui::Begin("Settings")) {
        ImGui::End();
        return;
    }

    for (const auto &[name, value] : Debug::GetBoolSettings()) {
        bool newValue = value;
        if (ImGui::Checkbox(name.c_str(), &newValue))
            Debug::SetSetting(name, newValue);
    }

    for (const auto &[name, value] : Debug::GetIntSettings()) {
        int newValue = value;
        if (ImGui::DragInt(name.c_str(), &newValue))
            Debug::SetSetting(name, newValue);
    }

    for (const auto &[name, value] : Debug::GetFloatSettings()) {
        float newValue = value;
        if (ImGui::DragFloat(name.c_str(), &newValue))
            Debug::SetSetting(name, newValue);
    }

    for (const auto &[name, value] : Debug::GetFloat2Settings()) {
        DirectX::XMFLOAT2 newValue = value;
        if (ImGui::DragFloat2(name.c_str(), &newValue.x))
            Debug::SetSetting(name, newValue);
    }

    for (const auto &[name, value] : Debug::GetFloat3Settings()) {
        DirectX::XMFLOAT3 newValue = value;
        if (ImGui::DragFloat3(name.c_str(), &newValue.x))
            Debug::SetSetting(name, newValue);
    }

    for (const auto &[name, value] : Debug::GetFloat4Settings()) {
        DirectX::XMFLOAT4 newValue = value;
        if (ImGui::DragFloat4(name.c_str(), &newValue.x))
            Debug::SetSetting(name, newValue);
    }

    ImGui::End();
}