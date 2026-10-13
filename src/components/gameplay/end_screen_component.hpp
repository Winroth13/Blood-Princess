#pragma once

#include "scene/component.hpp"

class EndScreenComponent : public Component {
    RegisterComponent(EndScreenComponent);

public:
    EndScreenComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}

private:
    void OnStart(const AppContext &context) override;
};