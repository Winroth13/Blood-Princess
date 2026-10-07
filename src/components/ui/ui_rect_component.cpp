#include "ui_rect_component.hpp"
#include "scene/entity.hpp"
#include "scene/scene.hpp"
#include "components/ui/ui_canvas_component.hpp"
#include "core/logging.hpp"

#undef min
#undef max

namespace {
    constexpr struct AnchorPresetValues {
        XMFLOAT2 topLeft;
        XMFLOAT2 bottomRight;
        XMFLOAT2 pivot;
    } ANCHOR_PRESETS[static_cast<size_t>(UIAnchor::COUNT)] = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}}, // TOP_LEFT
        {{0.5f, 0.0f}, {0.5f, 0.0f}, {0.5f, 0.0f}}, // TOP
        {{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}}, // TOP_RIGHT
        {{0.0f, 0.5f}, {0.0f, 0.5f}, {0.0f, 0.5f}}, // LEFT
        {{0.5f, 0.5f}, {0.5f, 0.5f}, {0.5f, 0.5f}}, // CENTRE
        {{1.0f, 0.5f}, {1.0f, 0.5f}, {1.0f, 0.5f}}, // RIGHT
        {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f}}, // BOTTOM_LEFT
        {{0.5f, 1.0f}, {0.5f, 1.0f}, {0.5f, 1.0f}}, // BOTTOM
        {{1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}}, // BOTTOM_RIGHT
        {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 0.0f}}, // STRETCHED_HORIZONTAL_TOP
        {{0.0f, 0.5f}, {1.0f, 0.5f}, {0.5f, 0.5f}}, // STRETCHED_HORIZONTAL_MIDDLE
        {{0.0f, 1.0f}, {1.0f, 1.0f}, {0.5f, 1.0f}}, // STRETCHED_HORIZONTAL_BOTTOM
        {{0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.5f}}, // STRETCHED_VERTICAL_LEFT
        {{0.5f, 0.0f}, {0.5f, 1.0f}, {0.5f, 0.5f}}, // STRETCHED_VERTICAL_MIDDLE
        {{1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.5f}}, // STRETCHED_VERTICAL_RIGHT
        {{0.0f, 0.0f}, {1.0f, 1.0f}, {0.5f, 0.5f}}  // STRETCH
    };
}

void UIRectComponent::UpdateRectangle() {
    UIContext &ui = this->GetOwner()->GetScene()->GetUI();
    float screenWidth = static_cast<float>(ui.GetWidth());
    float screenHeight = static_cast<float>(ui.GetHeight());

    if (UICanvasComponent *canvas = this->GetOwner()->GetComponent<UICanvasComponent>()) {
        const float canvasScale = canvas->GetScaleFactor(screenHeight);
        this->cachedScreenRect = {{0.0f, 0.0f}, {screenWidth, screenHeight}};
        this->cachedPixelScale = {canvasScale, canvasScale};
        return;
    }

    UIRect parentRect = {{0.0f, 0.0f}, {screenWidth, screenHeight}};
    XMFLOAT2 parentScale = {1.0f, 1.0f};

    Entity *parent = this->GetOwner()->GetParent();
    UIRectComponent *parentRectComponent = parent ? parent->GetComponent<UIRectComponent>() : nullptr;

    if (parentRectComponent) {
        parentRect = parentRectComponent->GetScreenRect();
        parentScale = parentRectComponent->GetPixelScale();
    }
    else {
        LogWarn("Why is there no canvas at the top of the hierarchy? Do better. (%s)", this->GetOwner()->GetName().c_str());
    }

    const XMFLOAT2 topLeft = {
        parentRect.topLeft.x + parentRect.size.x * this->anchorTopLeft.x,
        parentRect.topLeft.y + parentRect.size.y * this->anchorTopLeft.y
    };

    const XMFLOAT2 bottomRight = {
        parentRect.topLeft.x + parentRect.size.x * this->anchorBottomRight.x,
        parentRect.topLeft.y + parentRect.size.y * this->anchorBottomRight.y
    };

    const XMFLOAT2 rectSize = {
        std::max(0.0f, (bottomRight.x - topLeft.x + this->sizeDelta.x * parentScale.x) * this->scale.x),
        std::max(0.0f, (bottomRight.y - topLeft.y + this->sizeDelta.y * parentScale.y) * this->scale.y)
    };

    const XMFLOAT2 pivotPosition = {
        topLeft.x + (bottomRight.x - topLeft.x) * this->pivot.x + this->position.x * parentScale.x,
        topLeft.y + (bottomRight.y - topLeft.y) * this->pivot.y + this->position.y * parentScale.y
    };

    this->cachedScreenRect.topLeft = {
        pivotPosition.x - rectSize.x * this->pivot.x,
        pivotPosition.y - rectSize.y * this->pivot.y
    };
    this->cachedScreenRect.size = rectSize;

    this->cachedPixelScale = {
        parentScale.x * this->scale.x,
        parentScale.y * this->scale.y
    };
}

const UIRect &UIRectComponent::GetScreenRect() {
    const uint32_t frameIndex = this->GetOwner()->GetScene()->GetUI().GetFrameIndex();
    
    if (this->cachedFrameIndex != frameIndex) {
        this->cachedFrameIndex = frameIndex;

        this->UpdateRectangle();
    }

    return this->cachedScreenRect;
}

XMFLOAT2 UIRectComponent::GetPixelScale() {
    this->GetScreenRect();
    return this->cachedPixelScale;
}

void UIRectComponent::SetAnchorPreset(UIAnchor anchorPreset) {
    const int index = static_cast<int>(anchorPreset);
    if (index < 0 || index >= static_cast<int>(UIAnchor::COUNT)) {
        LogWarn("Invalid anchor preset '%d'", index);
        return;
    }

    const AnchorPresetValues &values = ANCHOR_PRESETS[index];
    this->anchorTopLeft = values.topLeft;
    this->anchorBottomRight = values.bottomRight;
    this->pivot = values.pivot;
}

void UIRectComponent::Place(UIAnchor anchorPreset, XMFLOAT2 position, XMFLOAT2 sizeDelta) {
    this->SetAnchorPreset(anchorPreset);
    this->position = position;
    this->sizeDelta = sizeDelta;
}