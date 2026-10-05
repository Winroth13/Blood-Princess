#pragma once

#include "ui/ui_draw_list.hpp"

#include <cstdint>

class UIContext {
    UIDrawList drawList{};

    uint32_t frameIndex = 0;
    uint32_t width = 0;
    uint32_t height = 0;

public:
    UIContext() = default;

    void BeginFrame(uint32_t width, uint32_t height);
    void Clear();

    UIDrawList &GetDrawList() { return this->drawList; }

    uint32_t GetFrameIndex() const { return this->frameIndex; }
    uint32_t GetWidth() const { return this->width; }
    uint32_t GetHeight() const { return this->height; }
};