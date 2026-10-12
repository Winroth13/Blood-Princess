#include "enemy_controller_component.hpp"
#include "components/core/transform_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include <DirectXMath.h>
#include "components/ai/target_component.hpp"
#include "scene/scene.hpp"
#include "events/event_bus.hpp"
#include "events/game_events.hpp"
#include "components/audio/sound_component.hpp"
#include "assets/types/sound.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/health_component.hpp"

#include <algorithm>

void EnemyControllerComponent::OnStart(const AppContext& context)
{
    if (!this->hitSound && context.assetManager) {
        this->hitSound = context.assetManager->Get<Sound>("assets/sounds/goblin_hit_sound.sound");
    }

    this->transform = this->GetOwner()->GetComponent<TransformComponent>();

    if (!this->transform) {
        LogWarn("EnemyControllerComponent on '%s' has no TransformComponent",
            this->GetOwner()->GetName().c_str());
    }

    if (context.aiManager) {
        this->aiManager = context.aiManager;
        this->aiManager->RegisterEnemy(this);
    }
    else {
        LogWarn("EnemyControllerComponent on '%s' could not find AIManager",
            this->GetOwner()->GetName().c_str());
    }

    this->attackHitConnection = EventBus::Subscribe<AttackHitEvent>(
        this, &EnemyControllerComponent::OnAttackHit
    );

    this->GetOwner()->GetScene()->AddEntityToGroup(this->groupName, this->GetOwner());
}

void EnemyControllerComponent::OnDestroy(const AppContext & context)
{
    if (this->attackHitConnection != 0)
    {
        EventBus::Unsubscribe<AttackHitEvent>(this->attackHitConnection);
        this->attackHitConnection = 0;
    }

    if (this->aiManager) {
        this->aiManager->QueueUnregister(this);
        this->aiManager = nullptr;
    }
    this->GetOwner()->GetScene()->RemoveEntityFromGroup(this->groupName, this->GetOwner());
}

void EnemyControllerComponent::ApplyKnockback(const DirectX::XMFLOAT3& direction, float force)
{
    const float totalForce = force * this->knockbackMultiplier;
    this->knockbackVelocity.x = direction.x * totalForce;
    this->knockbackVelocity.y = 0.0f;
    this->knockbackVelocity.z = direction.z * totalForce;
    this->knockbackStunTimer = this->knockbackStunDuration;
}

void EnemyControllerComponent::ResetKnockback()
{
    this->knockbackVelocity = { 0.0f, 0.0f, 0.0f };
    this->knockbackStunTimer = 0.0f;
}

void EnemyControllerComponent::OnAttackHit(const AttackHitEvent& event)
{
    if (event.victim != this->GetOwner() || event.knockbackForce <= 0.0f)
    {
        return;
    }

    if (!this->transform)
    {
        return;
    }

    DirectX::XMFLOAT3 myPosF = this->transform->GetWorldPosition();
    auto* soundComponent = this->GetOwner()->GetComponent<SoundComponent>();
    auto* enemyHealth = this->GetOwner()->GetComponent<Health>();
    if (soundComponent && (!enemyHealth || !enemyHealth->IsDead())) {
        if (this->hitSound) {
            soundComponent->SetSound(this->hitSound);
        }
        soundComponent->Play();
    }
    DirectX::XMVECTOR myPosV = DirectX::XMLoadFloat3(&myPosF);

    DirectX::XMVECTOR attackerPosV = DirectX::XMVectorZero();
    bool foundAttackerPos = false;

    if (event.attacker)
    {
        auto* attackerTransform = event.attacker->GetComponent<TransformComponent>();
        if (attackerTransform)
        {
            DirectX::XMFLOAT3 aPos = attackerTransform->GetWorldPosition();
            attackerPosV = DirectX::XMLoadFloat3(&aPos);
            foundAttackerPos = true;
        }
    }

    if (!foundAttackerPos)
    {
        Entity* playerEnt = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
        if (playerEnt)
        {
            auto* pTransform = playerEnt->GetComponent<TransformComponent>();
            if (pTransform)
            {
                DirectX::XMFLOAT3 pPos = pTransform->GetWorldPosition();
                attackerPosV = DirectX::XMLoadFloat3(&pPos);
                foundAttackerPos = true;
            }
        }
    }

    if (!foundAttackerPos)
    {
        attackerPosV = DirectX::XMLoadFloat3(&event.hitPoint);
    }

    DirectX::XMVECTOR knockbackDirV = DirectX::XMVectorSetY(DirectX::XMVectorSubtract(myPosV, attackerPosV), 0.0f);
    const float lenSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(knockbackDirV));

    if (lenSq > 1e-4f)
    {
        knockbackDirV = DirectX::XMVector3Normalize(knockbackDirV);
    }
    else
    {
        knockbackDirV = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    DirectX::XMFLOAT3 dir;
    DirectX::XMStoreFloat3(&dir, knockbackDirV);

    this->ApplyKnockback(dir, event.knockbackForce);
}

DirectX::XMFLOAT3 EnemyControllerComponent::GetPosition() const
{
    if (!this->transform)
        return { 0.0f, 0.0f, 0.0f };

    return this->transform->GetWorldPosition();
}

void EnemyControllerComponent::UpdateAI(float deltaTime, size_t myGridIndex,
    const SpatialGrid& grid)
{
    if (!this->transform) return;

    XMFLOAT3 myPosF = this->transform->GetWorldPosition();
    XMVECTOR myPos = XMLoadFloat3(&myPosF);

    XMVECTOR kbVel = XMLoadFloat3(&this->knockbackVelocity);
    const float kbSpeedSq = XMVectorGetX(XMVector3LengthSq(kbVel));
    const bool hasKnockback = kbSpeedSq > 0.001f;

    if (this->knockbackStunTimer > 0.0f)
    {
        this->knockbackStunTimer -= deltaTime;
    }

    if (hasKnockback)
    {
        myPos = XMVectorAdd(myPos, XMVectorScale(kbVel, deltaTime));

        const float decay = (std::max)(0.0f, 1.0f - this->knockbackDecay * deltaTime);
        kbVel = XMVectorScale(kbVel, decay);
        XMStoreFloat3(&this->knockbackVelocity, kbVel);
    }

    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    if (!playerEnt)
    {
        if (hasKnockback)
        {
            myPos = XMVectorSetY(myPos, myPosF.y);
            XMFLOAT3 outPos;
            XMStoreFloat3(&outPos, myPos);
            this->transform->SetWorldPosition(outPos);
        }
        return;
    }

    TransformComponent* playerT = playerEnt->GetComponent<TransformComponent>();
    if (!playerT)
    {
        if (hasKnockback)
        {
            myPos = XMVectorSetY(myPos, myPosF.y);
            XMFLOAT3 outPos;
            XMStoreFloat3(&outPos, myPos);
            this->transform->SetWorldPosition(outPos);
        }
        return;
    }

    XMFLOAT3 playerPosF = playerT->GetWorldPosition();
    XMVECTOR player = XMLoadFloat3(&playerPosF);

    XMVECTOR toPlayer = XMVectorSetY(XMVectorSubtract(player, myPos), 0.0f);
    const float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));

    XMVECTOR seek = XMVectorZero();

    if (distSq > 1e-4f)
    {
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, toPlayer);
        const float angleDeg = atan2f(dir.x, dir.z) * (180.0f / XM_PI);
        this->transform->SetWorldRotation(0.0f, angleDeg, 0.0f);

        if (this->knockbackStunTimer <= 0.0f && distSq > this->stopDistance * this->stopDistance)
        {
            seek = XMVector3Normalize(toPlayer);
        }
    }

    XMVECTOR avoid = XMVectorSetY(grid.ComputeAvoidance(myGridIndex,
        this->avoidanceRadius),
        0.0f);

    XMVECTOR steer = XMVectorSetY(XMVectorAdd(seek, avoid), 0.0f);

    const float steerLen = XMVectorGetX(XMVector3Length(steer));
    if (steerLen > 1e-4f)
    {
        float effectiveSteerLen = steerLen;
        if (steerLen > 1.0f)
        {
            steer = XMVectorScale(steer, 1.0f / steerLen);
            effectiveSteerLen = 1.0f;
        }

        const float distMoved = effectiveSteerLen * this->moveSpeed * deltaTime;
        myPos = XMVectorAdd(myPos,
            XMVectorScale(steer,
                this->moveSpeed * deltaTime));

        if (this->footstepSound && this->footstepIntervalMeters > 0.0f)
        {
            this->movementDistanceAccumulator += distMoved;
            if (this->movementDistanceAccumulator >= this->footstepIntervalMeters)
            {
                this->movementDistanceAccumulator = 0.0f;
                if (auto* soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
                {
                    soundComponent->SetSound(this->footstepSound);
                    soundComponent->Play();
                }
            }
        }
    }

    if (steerLen > 1e-4f || hasKnockback)
    {
        myPos = XMVectorSetY(myPos, myPosF.y);

        XMFLOAT3 outPos;
        XMStoreFloat3(&outPos, myPos);
        this->transform->SetWorldPosition(outPos);
    }
}