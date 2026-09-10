#pragma once

#include "scene/component.hpp"

class CameraComponent : public Component 
{
    float fieldOfView = XM_PIDIV2; // Radians
    float nearPlane = 0.1f;
    float farPlane = 512.0f;

public:
    CameraComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoRender(); }

    void OnStart(const AppContext &context) override {}
    void Update(const AppContext &context, float deltaTime) override;
    void Render(const RenderView &view, RenderQueue &queue) override {}
    void OnDestroy(const AppContext &context) override {}
};