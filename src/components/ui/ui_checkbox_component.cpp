#include "ui_checkbox_component.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "scene/entity.hpp"

UIInteractableComponent *UICheckboxComponent::GetInteractable() {
    Entity *owner = this->GetOwner();
    if (UIInteractableComponent *interactable = owner->GetComponent<UIInteractableComponent>())
        return interactable;

    return owner->AddComponent<UIInteractableComponent>();
}

bool UICheckboxComponent::IsInteractable() {
    return this->GetInteractable()->IsInteractable();
}

void UICheckboxComponent::SetInteractable(bool isInteractable) {
    this->GetInteractable()->SetInteractable(isInteractable);
}

void UICheckboxComponent::SetCheckMark(Entity *checkMark) {
    this->checkMark = checkMark;

    if (this->checkMark)
        this->checkMark->SetLocallyActive(this->isOn);
}

void UICheckboxComponent::SetOn(bool isOn, bool shouldNotify) {
    this->isOn = isOn;
    this->Apply(shouldNotify);
}

void UICheckboxComponent::Apply(bool shouldNotify) {
    if (this->checkMark)
        this->checkMark->SetLocallyActive(this->isOn);

    if (this->isOn == this->appliedIsOn)
        return;

    this->appliedIsOn = this->isOn;

    if (shouldNotify)
        this->onValueChanged.Emit(this->isOn);
}

void UICheckboxComponent::HandleClick() {
    if (this->IsActive())
        this->Toggle();
}

void UICheckboxComponent::OnStart(const AppContext &context) {
    this->Apply(false);

    this->GetInteractable()->GetOnClick().Connect(this, &UICheckboxComponent::HandleClick);
}

void UICheckboxComponent::Update(const AppContext &context, float deltaTime) {
    if (this->isOn != this->appliedIsOn)
        this->Apply(true);
}

void UICheckboxComponent::OnDestroy(const AppContext &context) {
    if (UIInteractableComponent *interactable = this->GetOwner()->GetComponent<UIInteractableComponent>())
        interactable->GetOnClick().DisconnectObject(this);
}
