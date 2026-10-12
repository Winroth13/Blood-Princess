#include "ui_context.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "scene/entity.hpp"

#include <vector>

namespace {
    UIInteractableComponent *FindInteractable(Entity *entity) {
        for (Entity *current = entity; current; current = current->GetParent()) {
            if (UIInteractableComponent *interactable = current->GetComponent<UIInteractableComponent>())
                return interactable->CanInteract() ? interactable : nullptr;
        }

        return nullptr;
    }
}

namespace UI {
    void Context::SetHovered(UIInteractableComponent *interactable) {
        if (interactable == this->hovered)
            return;

        UIInteractableComponent *previous = this->hovered;
        this->hovered = interactable;

        if (previous)
            previous->HandlePointerExit();

        if (this->hovered)
            this->hovered->HandlePointerEnter();
    }

    void Context::BeginFrame(uint32_t width, uint32_t height) {
        this->drawList.Clear();
        ++this->frameIndex;
        this->width = width;
        this->height = height;
    }

    void Context::ProcessInput(const PointerState &pointer) {
        UIInteractableComponent *target = nullptr;
        this->isPointerOverUI = false;

        if (pointer.isEnabled) {
            const std::vector<HitTarget> &hitTargets = this->drawList.GetHitTargets();

            for (auto iter = hitTargets.rbegin(); iter != hitTargets.rend(); ++iter) {
                if (!iter->rect.Contains(pointer.position))
                    continue;

                this->isPointerOverUI = true;
                target = FindInteractable(iter->entity);
                break;
            }
        }

        if (this->pressed && (!pointer.isEnabled || !this->pressed->CanInteract())) {
            UIInteractableComponent *released = this->pressed;
            this->pressed = nullptr;
            released->HandlePointerUp(pointer.position);
        }

        if (this->pressed && target != this->pressed)
            target = nullptr;

        this->SetHovered(target);

        if (!pointer.isEnabled)
            return;

        if (pointer.wasPressed && this->hovered && !this->pressed) {
            this->pressed = this->hovered;
            this->pressed->HandlePointerDown(pointer.position);
        }
        else if (this->pressed && pointer.isDown) {
            this->pressed->HandlePointerDrag(pointer.position);
        }

        if (this->pressed && (pointer.wasReleased || !pointer.isDown)) {
            UIInteractableComponent *released = this->pressed;
            this->pressed = nullptr;

            released->HandlePointerUp(pointer.position);

            if (released == this->hovered)
                released->HandleClick();
        }
    }

    void Context::ForgetInteractable(UIInteractableComponent *interactable) {
        if (this->hovered == interactable)
            this->hovered = nullptr;

        if (this->pressed == interactable)
            this->pressed = nullptr;
    }

    void Context::Clear() {
        this->drawList.Clear();
        ++this->frameIndex;

        this->hovered = nullptr;
        this->pressed = nullptr;
        this->isPointerOverUI = false;
    }
}
