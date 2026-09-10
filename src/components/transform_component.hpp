#pragma once

#include "scene/component.hpp"

class TransformComponent : public Component 
{
public:
    TransformComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}

    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override {}
};