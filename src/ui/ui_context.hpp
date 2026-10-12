#pragma once

#include "ui/ui_draw_list.hpp"

#include <DirectXMath.h>
#include <cstdint>

class UIInteractableComponent;

namespace UI {
    struct PointerState {
        DirectX::XMFLOAT2 position = {0.0f, 0.0f};
        bool isEnabled = false;
        bool isDown = false;
        bool wasPressed = false;
        bool wasReleased = false;
    };

    class Context {
        DrawList drawList{};

        uint32_t frameIndex = 0;
        uint32_t width = 0;
        uint32_t height = 0;

        UIInteractableComponent *hovered = nullptr;
        UIInteractableComponent *pressed = nullptr;
        bool isPointerOverUI = false;

        void SetHovered(UIInteractableComponent *interactable);

    public:
        Context() = default;

        void BeginFrame(uint32_t width, uint32_t height);
        void Clear();

        void ProcessInput(const PointerState &pointer);
        void ForgetInteractable(UIInteractableComponent *interactable);
        bool IsPointerOverUI() const { return this->isPointerOverUI || this->pressed != nullptr; }

        DrawList &GetDrawList() { return this->drawList; }

        uint32_t GetFrameIndex() const { return this->frameIndex; }
        uint32_t GetWidth() const { return this->width; }
        uint32_t GetHeight() const { return this->height; }
    };
}