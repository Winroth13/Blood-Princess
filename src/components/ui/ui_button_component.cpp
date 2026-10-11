#include "ui_button_component.hpp"
#include "components/ui/ui_image_component.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "scene/entity.hpp"
#include "scene/scene.hpp"

#include <algorithm>

#undef min
#undef max

namespace {
    XMFLOAT4 Lerp(XMFLOAT4 from, XMFLOAT4 to, float t) {
        return {
            from.x + (to.x - from.x) * t,
            from.y + (to.y - from.y) * t,
            from.z + (to.z - from.z) * t,
            from.w + (to.w - from.w) * t
        };
    }
}

UIInteractableComponent *UIButtonComponent::GetInteractable() {
    Entity *owner = this->GetOwner();
    if (UIInteractableComponent *interactable = owner->GetComponent<UIInteractableComponent>())
        return interactable;

    return owner->AddComponent<UIInteractableComponent>();
}

Signal<> &UIButtonComponent::GetOnClick() {
    return this->GetInteractable()->GetOnClick();
}

bool UIButtonComponent::IsInteractable() {
    return this->GetInteractable()->IsInteractable();
}

void UIButtonComponent::SetInteractable(bool isInteractable) {
    this->GetInteractable()->SetInteractable(isInteractable);
}

UIImageComponent *UIButtonComponent::GetTargetImage() {
    if (this->targetImage)
        return this->targetImage;

    return this->GetOwner()->GetComponent<UIImageComponent>();
}

void UIButtonComponent::SetTargetImage(UIImageComponent *targetImage) {
    this->targetImage = targetImage;

    this->normalTexture = nullptr;
    this->appliedTexture = nullptr;
    this->hasShownState = false;
}

UI::ButtonState UIButtonComponent::GetStateFromInteractable() {
    UIInteractableComponent *interactable = this->GetInteractable();

    if (!interactable->IsInteractable())
        return UI::ButtonState::DISABLED;

    if (interactable->IsPressed())
        return interactable->IsHovered() ? UI::ButtonState::PRESSED : UI::ButtonState::NORMAL;

    if (interactable->IsHovered())
        return UI::ButtonState::HOVERED;

    return UI::ButtonState::NORMAL;
}

XMFLOAT4 UIButtonComponent::GetStateColour(UI::ButtonState state) const {
    switch (state) {
        case UI::ButtonState::HOVERED: return this->hoveredColour;
        case UI::ButtonState::PRESSED: return this->pressedColour;
        case UI::ButtonState::DISABLED: return this->disabledColour;
        default: return this->normalColour;
    }
}

std::shared_ptr<Texture2D> UIButtonComponent::GetStateTexture(UI::ButtonState state) const {
    switch (state) {
        case UI::ButtonState::HOVERED: return this->hoveredTexture;
        case UI::ButtonState::PRESSED: return this->pressedTexture;
        case UI::ButtonState::DISABLED: return this->disabledTexture;
        default: return nullptr;
    }
}

void UIButtonComponent::OnStart(const AppContext &context) {
    this->GetInteractable();
}

void UIButtonComponent::Update(const AppContext &context, float deltaTime) {
    const UI::ButtonState newState = this->GetStateFromInteractable();
    const bool hasChanged = newState != this->state;

    const uint32_t frameIndex = this->GetOwner()->GetScene()->GetUI().GetFrameIndex();
    const bool shouldSnap = !this->hasShownState || frameIndex > this->lastFrameIndex + 1;
    this->lastFrameIndex = frameIndex;

    if (hasChanged || shouldSnap) {
        this->state = newState;
        this->fadeFrom = this->colour;
        this->fadeProgress = shouldSnap ? 1.0f : 0.0f;
    }

    if (this->fadeProgress < 1.0f)
        this->fadeProgress = this->fadeDuration > 0.0f ? std::min(1.0f, this->fadeProgress + deltaTime / this->fadeDuration) : 1.0f;

    this->colour = Lerp(this->fadeFrom, this->GetStateColour(this->state), this->fadeProgress);

    if (UIImageComponent *image = this->GetTargetImage()) {
        if (this->isTinted)
            image->SetColour(this->colour);

        const std::shared_ptr<Texture2D> currentTexture = image->GetTexture();
        if (currentTexture != this->appliedTexture)
            this->normalTexture = currentTexture;

        std::shared_ptr<Texture2D> texture = this->GetStateTexture(this->state);
        if (!texture)
            texture = this->normalTexture;

        if (texture != currentTexture)
            image->SetTexture(texture);

        this->appliedTexture = texture;
    }

    this->hasShownState = true;

    if (hasChanged)
        this->onStateChanged.Emit(this->state);
}