#pragma once
#include "scene/component.hpp"
#include "scene/entity.hpp"
#include <unordered_set>
#include "DirectXMath.h"
class HitboxComponent : public Component
{
    RegisterComponent(HitboxComponent);

    ExportProperty(float, damage, 20.0f);
    ExportProperty(float, knockbackForce, 5.0f);
    ExportProperty(float, activeDuration, 0.0f);

    float activeTimer = 0.0f;
    std::unordered_set<Entity*> hitEntities;


private:
    void Update(const AppContext& context, float deltaTime) override;

public:
    HitboxComponent(Entity* owner, bool isActive) : Component(owner, isActive) { NoLateUpdate(), NoRender(); }
    void Activate(float duration = 0.0f);
    void Deactivate();

    bool CanHit(Entity* target) const;
    void OnSuccessfulHit(Entity* victim, const DirectX::XMFLOAT3& hitPoint);

    float GetDamage() const { return this->damage; }
    void SetDamage(float damage) { this->damage = damage; }

    float GetKnockbackForce() const { return this->knockbackForce; }
    void SetKnockbackForce(float amount) { this->knockbackForce = amount; }



};
