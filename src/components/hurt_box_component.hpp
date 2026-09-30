#pragma once
#include "scene/component.hpp"
#include "components/hit_box_component.hpp"
#include "components/health_component.hpp"
#include "events/event_bus.hpp"
#include <DirectXMath.h>
#include "scene/entity.hpp"
#include "core/logging.hpp"
class HurtboxComponent : public Component
{
public:
    RegisterComponent(HurtboxComponent);

    ExportProperty(float, damageMultiplier, 1.0f);
    ExportProperty(bool, isInvulnerable, false);
    ExportProperty(float, invulnerabilityTimer, 0.0f);

    Health* cachedHealth = nullptr;
    ConnectionId collisionEventConn = 0;


private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;
public:
    HurtboxComponent(Entity* owner, bool isActive) : Component(owner, isActive) { NoLateUpdate(), NoRender(); }
    void OnCollision(const CollisionEvent& e);

    bool ReceiveHit(HitboxComponent* hitbox, const DirectX::XMFLOAT3& hitPoint, const DirectX::XMFLOAT3& hitNormal);
    void GrantInvulnerability(float duration);

    bool IsInvulnerable() const { return this->isInvulnerable || this->invulnerabilityTimer > 0.0f; };

    float GetDamageMultiplier() const { return this->damageMultiplier; }
    void SetDamageMultiplier(float mult) { this->damageMultiplier = mult; }
};
