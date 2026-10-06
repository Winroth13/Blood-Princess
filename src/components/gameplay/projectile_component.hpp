#pragma once
#include "scene/component.hpp"
#include "scene/entity.hpp"

class ProjectileComponent :
    public Component
{
public:
    RegisterComponent(ProjectileComponent);
    DirectX::XMFLOAT3 velocity = { 0,0,0 };
    DirectX::XMFLOAT3 angularVelocity = { 0,0,0 };
    float groundY = -2.5f;
    float gravity = 9.82f;
    float projectileAliveTime = 8.0f;

    bool isGrounded = false;

private:
    void Update(const AppContext& context, float deltaTime) override;
public:
    ProjectileComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

    void Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3 & angVel = { 0.0f, 0.0f, 0.0f });
    bool IsGrounded()const { return isGrounded; };

    float GetAliveTime() const { return projectileAliveTime; }
    void SetAliveTime(float amount) { this->projectileAliveTime = amount; }

    void SetGravity(float strength) { this->gravity = strength; }
};

