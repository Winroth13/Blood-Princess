#pragma once
#include "scene/component.hpp"
#include "scene/entity.hpp"

class HurtboxComponent : public Component
{
public:
    HurtboxComponent(Entity* owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(); }

private:
    void OnStart(const AppContext& context) override {}
    void Update(const AppContext& context, float deltaTime) override {}
    void LateUpdate(const AppContext& context, float deltaTime) override {}
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override {}
};
