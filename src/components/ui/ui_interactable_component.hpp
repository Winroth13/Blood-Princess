#pragma once

#include "scene/component.hpp"
#include "events/signal.hpp"

#include <DirectXMath.h>

using namespace DirectX;

namespace UI {
    class Context;
}

class UIInteractableComponent : public Component {
    RegisterComponent(UIInteractableComponent);

    ExportProperty(bool, isInteractable, true);

    bool isHovered = false;
    bool isPressed = false;

    Signal<> onPointerEnter;
    Signal<> onPointerExit;
    Signal<XMFLOAT2> onPointerDown;
    Signal<XMFLOAT2> onPointerDrag;
    Signal<XMFLOAT2> onPointerUp;
    Signal<> onClick;

    friend class UI::Context;
    void HandlePointerEnter();
    void HandlePointerExit();
    void HandlePointerDown(XMFLOAT2 position);
    void HandlePointerDrag(XMFLOAT2 position);
    void HandlePointerUp(XMFLOAT2 position);
    void HandleClick();

public:
    UIInteractableComponent(Entity *owner, bool isActive) : Component(owner, isActive) { }

private:
    void OnDestroy(const AppContext &context) override;

public:
    bool CanInteract();

    bool IsInteractable() const { return this->isInteractable; }
    void SetInteractable(bool isInteractable) { this->isInteractable = isInteractable; }

    bool IsHovered() const { return this->isHovered; }
    bool IsPressed() const { return this->isPressed; }

    Signal<> &GetOnPointerEnter() { return this->onPointerEnter; }
    Signal<> &GetOnPointerExit() { return this->onPointerExit; }
    Signal<XMFLOAT2> &GetOnPointerDown() { return this->onPointerDown; }
    Signal<XMFLOAT2> &GetOnPointerDrag() { return this->onPointerDrag; }
    Signal<XMFLOAT2> &GetOnPointerUp() { return this->onPointerUp; }
    Signal<> &GetOnClick() { return this->onClick; }
};
