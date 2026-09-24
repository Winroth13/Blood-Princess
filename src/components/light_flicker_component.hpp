#pragma once

#include "scene/entity.hpp"
#include "scene/component.hpp"
#include "events/signal.hpp"

class LightFlickerComponent : public Component
{
    RegisterComponent(LightFlickerComponent);

    ExportProperty(float, flickerSpeed, 10.0f);
    ExportProperty(float, flickerIntensity, 5.0f);

    float startIntensity = 0.0f;
    float timer = 0.0f;

public:
    LightFlickerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoRender(); NoLateUpdate(); }

private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext &context, float deltaTime) override;
    void LateUpdate(const AppContext& context, float deltaTime) override {}
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override {};

public:
};