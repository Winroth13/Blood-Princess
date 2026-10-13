#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"

class TransformComponent;
class HitboxComponent;

// Per-trap lifecycle state machine.
//   Telegraph  -> trap rises from riseHeight below rest, hitbox inactive
//   Armed      -> hitbox active, deals damage
//   Expired    -> spawner returns the entity to its pool
class TrapComponent : public Component
{
    RegisterComponent(TrapComponent);

    ExportProperty(float, telegraphDuration, 1.0f);
    ExportProperty(float, armedDuration, 3.0f);

    ExportProperty(float, riseHeight, 0.0f);

    DirectX::XMFLOAT3 restPosition = { 0.0f,0.0f,0.0f };
    float elapsedTime = 0.0f;
    bool isArmed = false;
    bool isExpired = false;

    TransformComponent* cachedTransform = nullptr;
    HitboxComponent* cachedHitbox = nullptr;

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;

public:
    TrapComponent(Entity* owner, bool isActive)
        : Component(owner, isActive) {
        UsesUpdate();
    }

    bool IsExpired() const { return this->isExpired; }
    bool IsArmed() const { return this->isArmed; }

    void SetTelegraph(float s) { this->telegraphDuration = s; }
    void SetArmed(float s) { this->armedDuration = s; }

    // Called by the spawner before re-activating a pooled trap.
    void Reset();
};