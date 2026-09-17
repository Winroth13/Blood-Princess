#pragma once
#include "scene/component.hpp"

class CollisionComponent : public Component
{
protected:
    CollisionComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); NoRender(); }

public:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void Render(const RenderView &view, RenderQueue &queue) override;
    void OnDestroy(const AppContext &context) override;
};
