#pragma once

class SettingsPanel {
public:
    SettingsPanel() = default;

    bool Initialize() { return true; };
    void ImGuiRender();
};