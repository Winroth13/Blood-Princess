#pragma once

#include "scene/component.hpp"

/*
*   A components that after it's given lifetime is either destroyed or
*   if owned by a pool is returned back to it.
*/

class LifeTimeComponent : public Component
{
    RegisterComponent(LifeTimeComponent);

    ExportPropertyRanged(float, lifeTime, "s", 0.0f, 0.0f, 0.01f, 5.0f);

public:
    LifeTimeComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void Update(const AppContext &context, float deltaTime) override;

public:
    float GetLifeTime() const { return this->lifeTime; }
    void SetLifeTime(const float lifeTime) { this->lifeTime = lifeTime; }
};
