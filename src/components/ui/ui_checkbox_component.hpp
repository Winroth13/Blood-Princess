#pragma once

#include "scene/component.hpp"
#include "events/signal.hpp"

class UIInteractableComponent;

class UICheckboxComponent : public Component {
    RegisterComponent(UICheckboxComponent);

    ExportProperty(bool, isOn, false);

    Entity *checkMark = nullptr;

    Signal<bool> onValueChanged;

    bool appliedIsOn = false;

    void Apply(bool shouldNotify);
    void HandleClick();

public:
    UICheckboxComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void OnDestroy(const AppContext &context) override;

public:
    UIInteractableComponent *GetInteractable();

    Signal<bool> &GetOnValueChanged() { return this->onValueChanged; }

    bool IsOn() const { return this->isOn; }
    void SetOn(bool isOn, bool shouldNotify = true);
    void Toggle() { this->SetOn(!this->isOn); }

    Entity *GetCheckMark() const { return this->checkMark; }
    void SetCheckMark(Entity *checkMark);

    bool IsInteractable();
    void SetInteractable(bool isInteractable);
};
