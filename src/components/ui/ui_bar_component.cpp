#include "ui_bar_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "assets/types/texture_2d.hpp"
#include "scene/entity.hpp"
#include "ui/ui_draw_list.hpp"

#include <algorithm>
#include <cmath>

#undef min
#undef max

namespace {
    inline bool HasBorder(XMFLOAT4 border) {
        return border.x > 0.0f || border.y > 0.0f || border.z > 0.0f || border.w > 0.0f;
    }
}

void UIBarComponent::OnFillDirectionChanged() {
    if (this->fillDirection_ == "LEFT_TO_RIGHT")
        this->fillDirection = UI::FillDirection::LEFT_TO_RIGHT;
    else if (this->fillDirection_ == "RIGHT_TO_LEFT")
        this->fillDirection = UI::FillDirection::RIGHT_TO_LEFT;
    else if (this->fillDirection_ == "TOP_TO_BOTTOM")
        this->fillDirection = UI::FillDirection::TOP_TO_BOTTOM;
    else if (this->fillDirection_ == "BOTTOM_TO_TOP")
        this->fillDirection = UI::FillDirection::BOTTOM_TO_TOP;
}

void UIBarComponent::SetFillDirection(UI::FillDirection fillDirection) {
    this->fillDirection = fillDirection;

    if (fillDirection == UI::FillDirection::LEFT_TO_RIGHT)
        this->fillDirection_ = "LEFT_TO_RIGHT";
    else if (fillDirection == UI::FillDirection::RIGHT_TO_LEFT)
        this->fillDirection_ = "RIGHT_TO_LEFT";
    else if (fillDirection == UI::FillDirection::TOP_TO_BOTTOM)
        this->fillDirection_ = "TOP_TO_BOTTOM";
    else if (fillDirection == UI::FillDirection::BOTTOM_TO_TOP)
        this->fillDirection_ = "BOTTOM_TO_TOP";
}

float UIBarComponent::GetFraction(float value) const {
    return std::clamp(this->maxValue <= 0.0f ? 0.0f : value / this->maxValue, 0.0f, 1.0f);
}

void UIBarComponent::OnStart(const AppContext &context) {
    this->trailValue = this->value;
    this->lastValue = this->value;
    this->hasTrailStarted = true;
}

void UIBarComponent::Update(const AppContext &context, float deltaTime) {
    if (!this->hasTrailStarted) {
        this->trailValue = this->value;
        this->lastValue = this->value;
        this->hasTrailStarted = true;
    }

    if (this->value < this->lastValue)
        this->trailTimer = this->trailDelay;

    this->lastValue = this->value;

    if (this->trailValue <= this->value) {
        this->trailValue = this->value;
        this->trailTimer = 0.0f;
    }
    else if (this->trailTimer > 0.0f)
        this->trailTimer -= deltaTime;
    else
        this->trailValue = std::max(this->value, this->trailValue - this->trailSpeed * this->maxValue * deltaTime);
}

void UIBarComponent::DrawFill(
    UI::DrawList &drawList,
    const UI::Rect &area,
    float fraction,
    const XMFLOAT4 &colour,
    const std::shared_ptr<Texture2D> &texture,
    const XMFLOAT4 &sliceBorder,
    XMFLOAT2 borderScale
) const {
    if (fraction <= 0.0f || colour.w <= 0.0f)
        return;

    XMFLOAT2 topLeft = area.topLeft;
    XMFLOAT2 bottomRight = area.BottomRight();
    XMFLOAT2 uvTopLeft = {0.0f, 0.0f};
    XMFLOAT2 uvBottomRight = {1.0f, 1.0f};

    switch (this->fillDirection) {
        case UI::FillDirection::LEFT_TO_RIGHT: {
            bottomRight.x = topLeft.x + area.size.x * fraction;
            uvBottomRight.x = fraction;
        } break;
        case UI::FillDirection::RIGHT_TO_LEFT: {
            topLeft.x = bottomRight.x - area.size.x * fraction;
            uvTopLeft.x = 1.0f - fraction;
        } break;
        case UI::FillDirection::TOP_TO_BOTTOM: {
            bottomRight.y = topLeft.y + area.size.y * fraction;
            uvBottomRight.y = fraction;
        } break;
        case UI::FillDirection::BOTTOM_TO_TOP: {
            topLeft.y = bottomRight.y - area.size.y * fraction;
            uvTopLeft.y = 1.0f - fraction;
        } break;
    }

    ID3D11ShaderResourceView *srv = texture ? texture->GetShaderResourceView() : nullptr;

    if (texture && HasBorder(sliceBorder)) {
        drawList.AddSlicedQuad(
            topLeft, 
            bottomRight, 
            {0.0f, 0.0f}, 
            {1.0f, 1.0f},
            sliceBorder,
            {static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight())},
            borderScale, 
            colour, 
            srv
        );
        return;
    }

    drawList.AddQuad(
        {std::round(topLeft.x), std::round(topLeft.y)},
        {std::round(bottomRight.x), std::round(bottomRight.y)},
        uvTopLeft, 
        uvBottomRight, 
        colour, 
        srv
    );
}

void UIBarComponent::UIRender(UI::DrawList &drawList) {
    UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>();
    if (!rectComponent)
        return;

    const UI::Rect &rect = rectComponent->GetScreenRect();
    const XMFLOAT2 pixelScale = rectComponent->GetPixelScale();
    const XMFLOAT2 borderScale = {this->sliceScale * pixelScale.x, this->sliceScale * pixelScale.y};

    if (this->isRaycastTarget)
        drawList.AddHitTarget(rect, this->GetOwner());

    if (this->backgroundColour.w > 0.0f) {
        ID3D11ShaderResourceView *srv = this->backgroundTexture ? this->backgroundTexture->GetShaderResourceView() : nullptr;

        if (this->backgroundTexture && HasBorder(this->backgroundSliceBorder)) {
            drawList.AddSlicedQuad(
                rect.topLeft, 
                rect.BottomRight(), 
                {0.0f, 0.0f}, 
                {1.0f, 1.0f},
                this->backgroundSliceBorder,
                {static_cast<float>(this->backgroundTexture->GetWidth()), static_cast<float>(this->backgroundTexture->GetHeight())},
                borderScale, 
                this->backgroundColour, 
                srv
            );
        }
        else {
            const XMFLOAT2 bottomRight = rect.BottomRight();
            drawList.AddQuad(
                {std::round(rect.topLeft.x), std::round(rect.topLeft.y)},
                {std::round(bottomRight.x), std::round(bottomRight.y)},
                {0.0f, 0.0f}, 
                {1.0f, 1.0f},
                this->backgroundColour, 
                srv
            );
        }
    }

    UI::Rect area = rect;
    area.topLeft.x += this->fillPadding.x * pixelScale.x;
    area.topLeft.y += this->fillPadding.y * pixelScale.y;
    area.size.x = std::max(0.0f, area.size.x - (this->fillPadding.x + this->fillPadding.z) * pixelScale.x);
    area.size.y = std::max(0.0f, area.size.y - (this->fillPadding.y + this->fillPadding.w) * pixelScale.y);

    if (this->trailColour.w > 0.0f && this->trailValue > this->value)
        this->DrawFill(drawList, area, this->GetFraction(this->trailValue), this->trailColour, this->fillTexture, this->fillSliceBorder, borderScale);

    this->DrawFill(drawList, area, this->GetFraction(this->value), this->fillColour, this->fillTexture, this->fillSliceBorder, borderScale);
}