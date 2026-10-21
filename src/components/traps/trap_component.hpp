#pragma once
#include "scene/component.hpp"
#include "events/event_bus.hpp"
#include "DirectXMath.h"

class TransformComponent;
class HitboxComponent;
class AnimatorComponent;
class SoundComponent;
class Sound;
enum class TrapState
{
    IDLE,
    TELEGRAPH,
    ARMED,
    COOLDOWN
};


// Per-trap lifecycle state machine.
//   Telegraph  -> trap rises from riseHeight below rest, hitbox inactive
//   Armed      -> hitbox active, deals damage
//   Expired    -> spawner returns the entity to its pool
class TrapComponent : public Component
{
    RegisterComponent(TrapComponent);

    ExportProperty(float, telegraphDuration, 0.4f);
    ExportProperty(float, armedDuration, 2.0f);
    ExportProperty(float, cooldownDuration, 2.0f);

    TrapState state = TrapState::IDLE;
    float timer = 0.0f;
    bool isExpired = false;

    ConnectionId collisionEventConn = 0;

    TransformComponent* cachedTransform = nullptr;
    HitboxComponent* cachedHitbox = nullptr;
    AnimatorComponent* cachedAnimator = nullptr;
    SoundComponent* cachedSound = nullptr;
    // sounds for trap

    std::shared_ptr<Sound> trapTelegraph;
    std::shared_ptr<Sound> trapArmed;
    std::shared_ptr<Sound> trapCooldown;

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;

    void OnCollision(const CollisionEvent& e);
    void TriggerArmed();

public:
    TrapComponent(Entity* owner, bool isActive)
        : Component(owner, isActive) {
        UsesUpdate();
    }

    bool IsExpired() const { return this->isExpired; }

    bool IsArmed() const { return this->state == TrapState::ARMED; }
    TrapState GetState() const { return this->state; }

    void SetTelegraph(float s) { this->telegraphDuration = s; }
    void SetArmed(float s) { this->armedDuration = s; }
    void SetCooldown(float s) { this->cooldownDuration = s; }

    // Called by the spawner before re-activating a pooled trap.
    void Reset();
};