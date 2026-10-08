#pragma once

class StatisticsPanel {
public:
    StatisticsPanel() = default;

    bool Initialize() { return true; };
    void ImGuiRender();
};