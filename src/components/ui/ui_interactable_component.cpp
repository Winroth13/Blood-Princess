#include "ui_interactable_component.hpp"
#include "scene/entity.hpp"
#include "scene/scene.hpp"

void UIInteractableComponent::OnDestroy(const AppContext &context) {
    if (Scene *scene = this->GetOwner()->GetScene())
        scene->GetUI().ForgetInteractable(this);
}

bool UIInteractableComponent::CanInteract() {
    return this->isInteractable && this->IsActive() && !this->IsPendingDestroy() && this->GetOwner()->IsActive();
}

void UIInteractableComponent::HandlePointerEnter() {
    this->isHovered = true;
    this->onPointerEnter.Emit();
}

void UIInteractableComponent::HandlePointerExit() {
    this->isHovered = false;
    this->onPointerExit.Emit();
}

void UIInteractableComponent::HandlePointerDown(XMFLOAT2 position) {
    this->isPressed = true;
    this->onPointerDown.Emit(position);
}

void UIInteractableComponent::HandlePointerDrag(XMFLOAT2 position) {
    this->onPointerDrag.Emit(position);
}

void UIInteractableComponent::HandlePointerUp(XMFLOAT2 position) {
    this->isPressed = false;
    this->onPointerUp.Emit(position);
}

void UIInteractableComponent::HandleClick() {
    if (this->CanInteract())
        this->onClick.Emit();
}
