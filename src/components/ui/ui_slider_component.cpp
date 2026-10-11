#include "ui_slider_component.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "components/ui/ui_bar_component.hpp"
#include "scene/entity.hpp"
#include "ui/ui_draw_list.hpp"

#include <algorithm>
#include <cmath>

#undef min
#undef max

UIInteractableComponent *UISliderComponent::GetInteractable() {
    Entity *owner = this->GetOwner();
    if (UIInteractableComponent *interactable = owner->GetComponent<UIInteractableComponent>())
        return interactable;

    return owner->AddComponent<UIInteractableComponent>();
}

bool UISliderComponent::IsInteractable() {
    return this->GetInteractable()->IsInteractable();
}

void UISliderComponent::SetInteractable(bool isInteractable) {
    this->GetInteractable()->SetInteractable(isInteractable);
}

void UISliderComponent::OnDirectionChanged() {
    if (this->direction_ == "LEFT_TO_RIGHT")
        this->direction = UI::FillDirection::LEFT_TO_RIGHT;
    else if (this->direction_ == "RIGHT_TO_LEFT")
        this->direction = UI::FillDirection::RIGHT_TO_LEFT;
    else if (this->direction_ == "TOP_TO_BOTTOM")
        this->direction = UI::FillDirection::TOP_TO_BOTTOM;
    else if (this->direction_ == "BOTTOM_TO_TOP")
        this->direction = UI::FillDirection::BOTTOM_TO_TOP;
}

void UISliderComponent::SetDirection(UI::FillDirection direction) {
    this->direction = direction;

    if (direction == UI::FillDirection::LEFT_TO_RIGHT)
        this->direction_ = "LEFT_TO_RIGHT";
    else if (direction == UI::FillDirection::RIGHT_TO_LEFT)
        this->direction_ = "RIGHT_TO_LEFT";
    else if (direction == UI::FillDirection::TOP_TO_BOTTOM)
        this->direction_ = "TOP_TO_BOTTOM";
    else if (direction == UI::FillDirection::BOTTOM_TO_TOP)
        this->direction_ = "BOTTOM_TO_TOP";
}

bool UISliderComponent::IsHorizontal() const {
    return this->direction == UI::FillDirection::LEFT_TO_RIGHT || this->direction == UI::FillDirection::RIGHT_TO_LEFT;
}

bool UISliderComponent::IsReversed() const {
    return this->direction == UI::FillDirection::RIGHT_TO_LEFT || this->direction == UI::FillDirection::BOTTOM_TO_TOP;
}

float UISliderComponent::Snap(float value) const {
    if (this->maxValue <= this->minValue)
        return this->minValue;

    value = std::clamp(value, this->minValue, this->maxValue);

    if (this->step > 0.0f)
        value = std::min(this->maxValue, this->minValue + std::round((value - this->minValue) / this->step) * this->step);

    return value;
}

float UISliderComponent::GetNormalizedValue() const {
    if (this->maxValue <= this->minValue)
        return 0.0f;

    return std::clamp((this->value - this->minValue) / (this->maxValue - this->minValue), 0.0f, 1.0f);
}

void UISliderComponent::SetRange(float minValue, float maxValue, float step) {
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->step = step;
    this->Apply(true);
}

void UISliderComponent::SetValue(float value, bool shouldNotify) {
    this->value = value;
    this->Apply(shouldNotify);
}

void UISliderComponent::Apply(bool shouldNotify) {
    this->value = this->Snap(this->value);
    this->UpdateVisuals();

    if (this->value == this->appliedValue)
        return;

    this->appliedValue = this->value;

    if (shouldNotify)
        this->onValueChanged.Emit(this->value);
}

UI::Rect UISliderComponent::GetTrack() {
    UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>();
    if (!rectComponent)
        return {};

    UI::Rect track = rectComponent->GetScreenRect();

    if (UIBarComponent *bar = this->GetOwner()->GetComponent<UIBarComponent>()) {
        const XMFLOAT4 padding = bar->GetFillPadding();
        const XMFLOAT2 pixelScale = rectComponent->GetPixelScale();

        track.topLeft.x += padding.x * pixelScale.x;
        track.topLeft.y += padding.y * pixelScale.y;
        track.size.x = std::max(0.0f, track.size.x - (padding.x + padding.z) * pixelScale.x);
        track.size.y = std::max(0.0f, track.size.y - (padding.y + padding.w) * pixelScale.y);
    }

    return track;
}

void UISliderComponent::SetValueFromPosition(XMFLOAT2 position) {
    const UI::Rect track = this->GetTrack();

    float along = 0.0f;
    if (this->IsHorizontal() && track.size.x > 0.0f)
        along = (position.x - this->grabOffset - track.topLeft.x) / track.size.x;
    else if (!this->IsHorizontal() && track.size.y > 0.0f)
        along = (position.y - this->grabOffset - track.topLeft.y) / track.size.y;

    along = std::clamp(along, 0.0f, 1.0f);
    const float normalized = this->IsReversed() ? 1.0f - along : along;

    this->SetValue(this->minValue + normalized * (this->maxValue - this->minValue));
}

void UISliderComponent::UpdateVisuals() {
    const float normalized = this->GetNormalizedValue();

    UIBarComponent *bar = this->GetOwner()->GetComponent<UIBarComponent>();
    if (bar) {
        bar->SetFillDirection(this->direction);
        bar->SetValue(normalized, 1.0f);
    }

    UIRectComponent *handleRect = this->handle ? this->handle->GetComponent<UIRectComponent>() : nullptr;
    if (!handleRect)
        return;

    const XMFLOAT4 padding = bar ? bar->GetFillPadding() : XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f};
    const float along = this->IsReversed() ? 1.0f - normalized : normalized;

    XMFLOAT2 anchor = {0.5f, 0.5f};
    XMFLOAT2 position = handleRect->GetPosition();

    if (this->IsHorizontal()) {
        anchor.x = along;
        position.x = padding.x * (1.0f - along) - padding.z * along;
    }
    else {
        anchor.y = along;
        position.y = padding.y * (1.0f - along) - padding.w * along;
    }

    handleRect->SetAnchorTopLeft(anchor);
    handleRect->SetAnchorBottomRight(anchor);
    handleRect->SetPivot({0.5f, 0.5f});
    handleRect->SetPosition(position);
}

void UISliderComponent::HandlePointerDown(XMFLOAT2 position) {
    if (!this->IsActive())
        return;

    this->grabOffset = 0.0f;

    if (UIRectComponent *handleRect = this->handle ? this->handle->GetComponent<UIRectComponent>() : nullptr) {
        const UI::Rect &rect = handleRect->GetScreenRect();

        if (this->handle->IsActive() && rect.Contains(position)) {
            this->grabOffset = this->IsHorizontal()
                ? position.x - (rect.topLeft.x + rect.size.x * 0.5f)
                : position.y - (rect.topLeft.y + rect.size.y * 0.5f);
        }
    }

    this->SetValueFromPosition(position);
}

void UISliderComponent::HandlePointerDrag(XMFLOAT2 position) {
    if (this->IsActive())
        this->SetValueFromPosition(position);
}

void UISliderComponent::OnStart(const AppContext &context) {
    this->Apply(false);

    UIInteractableComponent *interactable = this->GetInteractable();
    interactable->GetOnPointerDown().Connect(this, &UISliderComponent::HandlePointerDown);
    interactable->GetOnPointerDrag().Connect(this, &UISliderComponent::HandlePointerDrag);
}

void UISliderComponent::Update(const AppContext &context, float deltaTime) {
    this->Apply(true);
}

void UISliderComponent::UIRender(UI::DrawList &drawList) {
    if (!this->isRaycastTarget)
        return;

    if (UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>())
        drawList.AddHitTarget(rectComponent->GetScreenRect(), this->GetOwner());
}

void UISliderComponent::OnDestroy(const AppContext &context) {
    if (UIInteractableComponent *interactable = this->GetOwner()->GetComponent<UIInteractableComponent>()) {
        interactable->GetOnPointerDown().DisconnectObject(this);
        interactable->GetOnPointerDrag().DisconnectObject(this);
    }
}
