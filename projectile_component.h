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
    float groundY = 0.0f;
    float gravity = 9.82f;


    bool isGrounded = false;

private:
    void Update(const AppContext& context, float deltaTime) override;
public:
    ProjectileComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

    void Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3 & angVel = { 0.0f, 0.0f, 0.0f });
    bool IsGrounded()const { return isGrounded; };
};

