#pragma once

#include "scene/component.hpp"
#include "ui/ui_types.hpp"
#include "assets/types/texture_2d.hpp"

#include <DirectXMath.h>
#include <memory>
#include <string>

class UIBarComponent : public Component {
    RegisterComponent(UIBarComponent);

    ExportProperty(float, value, 100.0f);
    ExportProperty(float, maxValue, 100.0f);

    UI::FillDirection fillDirection = UI::FillDirection::LEFT_TO_RIGHT;
    ExportProperty(std::string, fillDirection_, "LEFT_TO_RIGHT");
    ExportPropertyChanged(fillDirection_, OnFillDirectionChanged);
    void OnFillDirectionChanged();

    ExportProperty(XMFLOAT4, fillPadding, {0.0f, 0.0f, 0.0f, 0.0f});

    ExportPropertyRanged(float, sliceScale, "x", 0.01f, 10.0f, 0.01f, 1.0f);

    ExportProperty(bool, isRaycastTarget, false);

    ExportPropertyCategory("Background");
    ExportProperty(std::shared_ptr<Texture2D>, backgroundTexture, nullptr);
    ExportPropertyColor(XMFLOAT4, backgroundColour, {1.0f, 1.0f, 1.0f, 1.0f});
    ExportProperty(XMFLOAT4, backgroundSliceBorder, {0.0f, 0.0f, 0.0f, 0.0f});

    ExportPropertyCategory("Fill");
    ExportProperty(std::shared_ptr<Texture2D>, fillTexture, nullptr);
    ExportPropertyColor(XMFLOAT4, fillColour, {1.0f, 1.0f, 1.0f, 1.0f});
    ExportProperty(XMFLOAT4, fillSliceBorder, {0.0f, 0.0f, 0.0f, 0.0f});

    ExportPropertyCategory("Trail");
    ExportPropertyColor(XMFLOAT4, trailColour, {0.8f, 0.8f, 0.8f, 1.0f});
    ExportPropertyRanged(float, trailDelay, "s", 0.0f, 5.0f, 0.01f, 0.5f);
    ExportPropertyRanged(float, trailSpeed, "/s", 0.01f, 10.0f, 0.01f, 0.6f);

    float trailValue = 0.0f;
    float trailTimer = 0.0f;
    float lastValue = 0.0f;
    bool hasTrailStarted = false;

    float GetFraction(float value) const;

    void DrawFill(
        UI::DrawList &drawList,
        const UI::Rect &area,
        float fraction,
        const XMFLOAT4 &colour,
        const std::shared_ptr<Texture2D> &texture,
        const XMFLOAT4 &sliceBorder,
        XMFLOAT2 borderScale
    ) const;

public:
    UIBarComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(), UsesUIRender(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void UIRender(UI::DrawList &drawList) override;

public:
    float GetValue() const { return this->value; }
    void SetValue(float value) { this->value = value; }
    void SetValue(float value, float maxValue) { this->value = value; this->maxValue = maxValue; }

    float GetMaxValue() const { return this->maxValue; }
    void SetMaxValue(float maxValue) { this->maxValue = maxValue; }

    float GetFraction() const { return this->GetFraction(this->value); }

    UI::FillDirection GetFillDirection() const { return this->fillDirection; }
    void SetFillDirection(UI::FillDirection fillDirection);

    XMFLOAT4 GetFillPadding() const { return this->fillPadding; }
    void SetFillPadding(XMFLOAT4 fillPadding) { this->fillPadding = fillPadding; }
    void SetFillPadding(float padding) { this->fillPadding = {padding, padding, padding, padding}; }

    bool IsRaycastTarget() const { return this->isRaycastTarget; }
    void SetRaycastTarget(bool isRaycastTarget) { this->isRaycastTarget = isRaycastTarget; }

    float GetSliceScale() const { return this->sliceScale; }
    void SetSliceScale(float sliceScale) { this->sliceScale = sliceScale; }

    std::shared_ptr<Texture2D> GetBackgroundTexture() const { return this->backgroundTexture; }
    void SetBackgroundTexture(std::shared_ptr<Texture2D> backgroundTexture) { this->backgroundTexture = backgroundTexture; }

    XMFLOAT4 GetBackgroundColour() const { return this->backgroundColour; }
    void SetBackgroundColour(XMFLOAT4 backgroundColour) { this->backgroundColour = backgroundColour; }

    XMFLOAT4 GetBackgroundSliceBorder() const { return this->backgroundSliceBorder; }
    void SetBackgroundSliceBorder(XMFLOAT4 backgroundSliceBorder) { this->backgroundSliceBorder = backgroundSliceBorder; }
    void SetBackgroundSliceBorder(float border) { this->backgroundSliceBorder = {border, border, border, border}; }

    std::shared_ptr<Texture2D> GetFillTexture() const { return this->fillTexture; }
    void SetFillTexture(std::shared_ptr<Texture2D> fillTexture) { this->fillTexture = fillTexture; }

    XMFLOAT4 GetFillColour() const { return this->fillColour; }
    void SetFillColour(XMFLOAT4 fillColour) { this->fillColour = fillColour; }

    XMFLOAT4 GetFillSliceBorder() const { return this->fillSliceBorder; }
    void SetFillSliceBorder(XMFLOAT4 fillSliceBorder) { this->fillSliceBorder = fillSliceBorder; }
    void SetFillSliceBorder(float border) { this->fillSliceBorder = {border, border, border, border}; }

    XMFLOAT4 GetTrailColour() const { return this->trailColour; }
    void SetTrailColour(XMFLOAT4 trailColour) { this->trailColour = trailColour; }

    float GetTrailDelay() const { return this->trailDelay; }
    void SetTrailDelay(float trailDelay) { this->trailDelay = trailDelay; }

    float GetTrailSpeed() const { return this->trailSpeed; }
    void SetTrailSpeed(float trailSpeed) { this->trailSpeed = trailSpeed; }

    float GetTrailValue() const { return this->trailValue; }
};