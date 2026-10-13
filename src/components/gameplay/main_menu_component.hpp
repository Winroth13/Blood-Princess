#pragma once

#include "scene/component.hpp"

class MainMenuComponent : public Component {
    RegisterComponent(MainMenuComponent);

public:
    MainMenuComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}

private:
    void OnStart(const AppContext &context) override;
};