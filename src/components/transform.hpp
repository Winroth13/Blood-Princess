#pragma once

#include "scene/component.hpp"

class Transform : public Component {
public:
    Transform(Entity *owner, bool isActive) : Component(owner, isActive) {}

    void OnStart(const AppContext &context) override {}
    void Update(const AppContext &context, float deltaTime) override;
    void Render() override {}
    void OnDestroy(const AppContext &context) override {}
};