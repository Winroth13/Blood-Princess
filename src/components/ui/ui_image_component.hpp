#pragma once

#include "scene/component.hpp"
#include "ui/ui_types.hpp"
#include "assets/types/texture_2d.hpp"

#include <DirectXMath.h>
#include <memory>

using namespace DirectX;

class UIImageComponent : public Component {
    RegisterComponent(UIImageComponent);

    ExportProperty(std::shared_ptr<Texture2D>, texture, nullptr);
    ExportProperty(XMFLOAT2, uvTopLeft, {0.0f, 0.0f});
    ExportProperty(XMFLOAT2, uvBottomRight, {1.0f, 1.0f});

    ExportPropertyColor(XMFLOAT4, colour, { 1.0f, 1.0f, 1.0f, 1.0f });

    ExportProperty(bool, shouldPreserveAspectRatio, false);

    UI::Rect GetPreservedAspectRatioRect(const UI::Rect &rect) const;

public:
    UIImageComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUIRender(); }

private:
    void UIRender(UIDrawList &drawList) override;

public:
    std::shared_ptr<Texture2D> GetTexture() const { return this->texture; }
    void SetTexture(std::shared_ptr<Texture2D> texture) { this->texture = texture; }

    XMFLOAT2 GetUVTopLeft() const { return this->uvTopLeft; }
    void SetUVTopLeft(XMFLOAT2 uvTopLeft) { this->uvTopLeft = uvTopLeft; }

    XMFLOAT2 GetUVBottomRight() const { return this->uvBottomRight; }
    void SetUVBottomRight(XMFLOAT2 uvBottomRight) { this->uvBottomRight = uvBottomRight; }

    const XMFLOAT4 &GetColour() const { return this->colour; }
    void SetColour(const XMFLOAT4 &colour) { this->colour = colour; }

    bool GetShouldPreserveAspectRatio() const { return this->shouldPreserveAspectRatio; }
    void SetShouldPreserveAspectRatio(bool shouldPreserveAspectRatio) { this->shouldPreserveAspectRatio = shouldPreserveAspectRatio; }
};