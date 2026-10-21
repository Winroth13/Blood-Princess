#pragma once

#include "scene/component.hpp"

class MenuScreenComponent : public Component {
    RegisterComponent(MenuScreenComponent);

public:
    MenuScreenComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}

private:
    void OnStart(const AppContext &context) override;
};