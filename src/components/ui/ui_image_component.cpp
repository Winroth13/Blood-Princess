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

void UIImageComponent::UIRender(UIDrawList &drawList) {
    UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>();
    if (!rectComponent)
        return;

    const UI::Rect &rect = rectComponent->GetScreenRect();

    if (this->colour.w <= 0.0f)
        return;

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