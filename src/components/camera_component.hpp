#pragma once

#include "scene/component.hpp"

class CameraComponent : public Component 
{
    RegisterComponent(CameraComponent)

    ExportPropertyClampedAngle(float, fieldOfView, 0.01f, XM_PI, XM_PIDIV2); // Radians

    ExportPropertyCategory("Plane");
    ExportPropertyRanged(float, nearPlane, "m", 0.01f, 5000.0f, 0.01f, 0.1f);
    ExportPropertyRanged(float, farPlane, "m", 1.0f, 5000.0f, 1, 512.0f);

    ExportPropertyCategory("Movement");
    ExportPropertyRanged(float, walkSpeed, "m/s", 0.01f, 5000.0f, 1, 2.0);
    ExportPropertyRanged(float, runSpeed, "m/s", 1.0f, 5000.0f, 1, 5.0f);

    ExportPropertyCategory("Combat");
    ExportProperty(float, health, 250.0f);
    ExportProperty(float, damage, 25.0);

public:
    CameraComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoRender(); }

    void OnStart(const AppContext &context) override {}
    void Update(const AppContext &context, float deltaTime) override;
    void Render(const RenderView &view, RenderQueue &queue) override {}
    void OnDestroy(const AppContext &context) override {}
};