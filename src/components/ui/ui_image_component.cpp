#include "ui_image_component.hpp"
#include "assets/types/texture_2d.hpp"
#include "ui/ui_draw_list.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "scene/entity.hpp"

#include <algorithm>
#include <cmath>

UI::Rect UIImageComponent::GetPreservedAspectRatioRect(const UI::Rect &rect) const {
    if (!this->shouldPreserveAspectRatio || !this->texture || rect.size.x <= 0.0f || rect.size.y <= 0.0f)
        return rect;

    const float spriteWidth = this->texture->GetWidth() * (this->uvBottomRight.x - this->uvTopLeft.x);
    const float spriteHeight = this->texture->GetHeight() * (this->uvBottomRight.y - this->uvTopLeft.y);
    if (spriteWidth <= 0.0f || spriteHeight <= 0.0f)
        return rect;

    const float spriteAspectRatio = spriteWidth / spriteHeight;
    const float rectAspectRatio = rect.size.x / rect.size.y;

    UI::Rect result = rect;
    if (spriteAspectRatio > rectAspectRatio)
        result.size.y = rect.size.x / spriteAspectRatio;
    else
        result.size.x = rect.size.y * spriteAspectRatio;

    result.topLeft.x += (rect.size.x - result.size.x) / 2;
    result.topLeft.y += (rect.size.y - result.size.y) / 2;

    return result;
}

void UIImageComponent::UIRender(UI::DrawList &drawList) {
    UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>();
    if (!rectComponent)
        return;

    const UI::Rect &rect = rectComponent->GetScreenRect();

    if (this->isRaycastTarget)
        drawList.AddHitTarget(rect, this->GetOwner());

    if (this->colour.w <= 0.0f)
        return;

    if (this->isSliced && this->texture) {
        const XMFLOAT2 pixelScale = rectComponent->GetPixelScale();

        drawList.AddSlicedQuad(
            rect.topLeft,
            rect.BottomRight(),
            this->uvTopLeft,
            this->uvBottomRight,
            this->sliceBorder,
            {static_cast<float>(this->texture->GetWidth()), static_cast<float>(this->texture->GetHeight())},
            {this->sliceScale * pixelScale.x, this->sliceScale * pixelScale.y},
            this->colour,
            this->texture->GetShaderResourceView()
        );
        return;
    }

    const UI::Rect drawRect = this->GetPreservedAspectRatioRect(rect);
    const XMFLOAT2 bottomRight = drawRect.BottomRight();

    drawList.AddQuad(
        {std::round(drawRect.topLeft.x), std::round(drawRect.topLeft.y)},
        {std::round(bottomRight.x), std::round(bottomRight.y)},
        this->uvTopLeft,
        this->uvBottomRight,
        this->colour,
        this->texture ? this->texture->GetShaderResourceView() : nullptr
    );
}

XMFLOAT2 UIImageComponent::GetSpriteSize() const {
    if (!this->texture)
        return {0.0f, 0.0f};

    return {
        this->texture->GetWidth() * std::abs(this->uvBottomRight.x - this->uvTopLeft.x),
        this->texture->GetHeight() * std::abs(this->uvBottomRight.y - this->uvTopLeft.y)
    };
}

float UIImageComponent::GetAspectRatio() const {
    const XMFLOAT2 spriteSize = this->GetSpriteSize();
    return (spriteSize.x > 0.0f && spriteSize.y > 0.0f) ? (spriteSize.x / spriteSize.y) : 0.0f;
}