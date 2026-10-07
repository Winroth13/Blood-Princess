#pragma once

#include "scene/component.hpp"
#include "ui/ui_types.hpp"

#include <DirectXMath.h>

using namespace DirectX;

class UIRectComponent : public Component {
    RegisterComponent(UIRectComponent);

    ExportProperty(XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, anchorBottomRight, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, position, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, sizeDelta, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, pivot, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, scale, {1.0f, 1.0f});

    UIRect cachedScreenRect{};
    XMFLOAT2 cachedPixelScale = {1.0f, 1.0f};
    uint32_t cachedFrameIndex = UINT32_MAX;

    void UpdateRectangle();

public:
    UIRectComponent(Entity *owner, bool isActive) : Component(owner, isActive) { }

    const UIRect &GetScreenRect();
    XMFLOAT2 GetPixelScale();

    bool ContainsPoint(XMFLOAT2 point) { return this->GetScreenRect().Contains(point); }

    void SetAnchorPreset(UIAnchor anchorPreset);
    // SetAnchorPreset() + SetPosition() + SetSize()
    void Place(UIAnchor anchorPreset, XMFLOAT2 position, XMFLOAT2 sizeDelta);

    XMFLOAT2 GetAnchorTopLeft() const { return this->anchorTopLeft; }
    void SetAnchorTopLeft(XMFLOAT2 anchorTopLeft) { this->anchorTopLeft = anchorTopLeft; }

    XMFLOAT2 GetAnchorBottomRight() const { return this->anchorBottomRight; }
    void SetAnchorBottomRight(XMFLOAT2 anchorBottomRight) { this->anchorBottomRight = anchorBottomRight; }

    XMFLOAT2 GetPosition() const { return this->position; }
    void SetPosition(XMFLOAT2 position) { this->position = position; }

    XMFLOAT2 GetSizeDelta() const { return this->sizeDelta; }
    void SetSizeDelta(XMFLOAT2 sizeDelta) { this->sizeDelta = sizeDelta; }

    XMFLOAT2 GetPivot() const { return this->pivot; }
    void SetPivot(XMFLOAT2 pivot) { this->pivot = pivot; }

    XMFLOAT2 GetScale() const { return this->scale; }
    void SetScale(XMFLOAT2 scale) { this->scale = scale; }
};