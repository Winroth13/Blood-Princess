#pragma once

#include "scene/component.hpp"

class CameraComponent : public Component 
{
    RegisterComponent(CameraComponent);

    ExportPropertyClampedAngle(float, fieldOfView, 0.01f, XM_PI, XM_PIDIV2); // Radians
    ExportPropertyRanged(float, nearPlane, "m", 0.01f, 5000.0f, 0.01f, 0.1f);
    ExportPropertyRanged(float, farPlane, "m", 1.0f, 5000.0f, 1, 15.0f);

public:
    CameraComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoLateUpdate(), NoRender(); }

private:
    void Update(const AppContext &context, float deltaTime) override;
};