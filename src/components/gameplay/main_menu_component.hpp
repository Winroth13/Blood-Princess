#pragma once

#include "scene/component.hpp"

class MainMenuComponent : public Component {
    RegisterComponent(MainMenuComponent);

public:
    MainMenuComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void Update(const AppContext& context, float deltaTime) override;
};