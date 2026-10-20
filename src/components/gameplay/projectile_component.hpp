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
    float friction = 50.0f;
    float restitution = 0.4f; // bounciness(0 = dead, 1 = lossless)

    float groundY = 0.0f;
    float gravity = 9.82f;
    float projectileAliveTime = 8.0f;

    float settleSpeedSq = 0.05f;  // on ground contact, stop when total speed below this

    bool isGrounded = false;
    bool destroyOnGrounded = false;
    bool hasCustomGroundY = false;

private:
    void Update(const AppContext& context, float deltaTime) override;
public:
    ProjectileComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

    void Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3 & angVel = { 0.0f, 0.0f, 0.0f });
    bool IsGrounded()const { return isGrounded; };

    float GetAliveTime() const { return projectileAliveTime; }

    void SetAliveTime(float amount) { this->projectileAliveTime = amount; }
    void SetGravity(float strength) { this->gravity = strength; }
    void SetRestitution(float r) { this->restitution = r; }
    void SetFriction(float f) { this->friction = f; }
    void SetGroundY(float y) { this->groundY = y; this->hasCustomGroundY = true; }
    void SetDestroyOnGrounded(bool destroy) { this->destroyOnGrounded = destroy; }

    XMFLOAT3 GetVelocity() const { return this->velocity; }
};

