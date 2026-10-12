#pragma once

#include "scene/component.hpp"
#include "ui/ui_types.hpp"
#include "events/signal.hpp"

#include <DirectXMath.h>
#include <string>

using namespace DirectX;

class UIInteractableComponent;

class UISliderComponent : public Component {
    RegisterComponent(UISliderComponent);

    ExportProperty(float, value, 0.5f);
    ExportProperty(float, minValue, 0.0f);
    ExportProperty(float, maxValue, 1.0f);
    ExportProperty(float, step, 0.0f);

    UI::FillDirection direction = UI::FillDirection::LEFT_TO_RIGHT;
    ExportProperty(std::string, direction_, "LEFT_TO_RIGHT");
    ExportPropertyChanged(direction_, OnDirectionChanged);
    void OnDirectionChanged();

    ExportProperty(bool, isRaycastTarget, true);

    Entity *handle = nullptr;

    Signal<float> onValueChanged;

    float appliedValue = 0.0f;
    float grabOffset = 0.0f;

    bool IsHorizontal() const;
    bool IsReversed() const;

    UI::Rect GetTrack();
    float Snap(float value) const;
    void SetValueFromPosition(XMFLOAT2 position);

    void Apply(bool shouldNotify);
    void UpdateVisuals();

    void HandlePointerDown(XMFLOAT2 position);
    void HandlePointerDrag(XMFLOAT2 position);

public:
    UISliderComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(), UsesUIRender(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void UIRender(UI::DrawList &drawList) override;
    void OnDestroy(const AppContext &context) override;

public:
    UIInteractableComponent *GetInteractable();

    Signal<float> &GetOnValueChanged() { return this->onValueChanged; }

    float GetValue() const { return this->value; }
    void SetValue(float value, bool shouldNotify = true);

    float GetNormalizedValue() const;

    float GetMinValue() const { return this->minValue; }
    float GetMaxValue() const { return this->maxValue; }
    float GetStep() const { return this->step; }
    void SetRange(float minValue, float maxValue, float step = 0.0f);

    UI::FillDirection GetDirection() const { return this->direction; }
    void SetDirection(UI::FillDirection direction);

    Entity *GetHandle() const { return this->handle; }
    void SetHandle(Entity *handle) { this->handle = handle; }

    bool IsRaycastTarget() const { return this->isRaycastTarget; }
    void SetRaycastTarget(bool isRaycastTarget) { this->isRaycastTarget = isRaycastTarget; }

    bool IsInteractable();
    void SetInteractable(bool isInteractable);
};
