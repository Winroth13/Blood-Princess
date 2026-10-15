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
#include "components/skeletal/animator_component.hpp"
#include "core/rng.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/core/model_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include <algorithm>

uint32_t s_arrowCounter = 0;

void EnemyControllerComponent::OnStart(const AppContext& context)
{
    if (!this->hitSound && context.assetManager) {
        this->hitSound = context.assetManager->Get<Sound>("assets/sounds/goblin_hit_sound.sound");
    }

    this->randomSoundTimer = RNG::Float(0.0f, 15.0f);

    this->transform = this->GetOwner()->GetComponent<TransformComponent>();
    this->animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    this->lastFootstepAnimTime = this->animator ? this->animator->GetTime() : -1.0f;

    this->SetEnemyType(this->enemyType);


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

    this->thisScene = this->GetOwner()->GetScene();
    this->arrowModel = context.assetManager->Get<Model>("assets/models/arrow.glb");
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
    if (totalForce <= 0.0f)
    {
        return;
    }
    this->knockbackVelocity.x = direction.x * totalForce;
    this->knockbackVelocity.y = 0.0f;
    this->knockbackVelocity.z = direction.z * totalForce;
    this->knockbackStunTimer = this->knockbackStunDuration;
}

void EnemyControllerComponent::ResetKnockback()
{
    this->knockbackVelocity = { 0.0f, 0.0f, 0.0f };
    this->knockbackStunTimer = 0.0f;
    this->lastFootstepAnimTime = this->animator ? this->animator->GetTime() : -1.0f;
    this->randomSoundTimer = RNG::Float(0.0f, 15.0f);
    this->isAttacking = false;

    this->attackElapsed = 0.0f;
    this->hitboxActivated = false;

    if (this->hitbox)
        this->hitbox->Deactivate();
}

void EnemyControllerComponent::SetEnemyType(EnemyType type)
{
    this->enemyType = type;

    const EnemyStats s = GetEnemyStats(type);

    this->moveSpeed = s.moveSpeed;
    this->stopDistance = s.stopDistance;
    this->knockbackMultiplier = s.knockbackMultiplier;
    this->attackDamage = s.attackDamage;
    this->attackKnockback = s.attackKnockback;
    this->attackHitboxDelay = s.attackHitboxDelay;
    this->attackHitboxDuration = s.attackHitboxDuration;

    if (this->hitbox)
    {
        this->hitbox->SetDamage(s.attackDamage);
        this->hitbox->SetKnockbackForce(s.attackKnockback);
    }
}

void EnemyControllerComponent::ShootAnArrow(XMFLOAT3 position)
{
    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");


    Entity* arrowEnt = this->thisScene->AddEntity("Arrow_" + std::to_string(s_arrowCounter++));

    TransformComponent* transform = arrowEnt->AddComponent<TransformComponent>();
    transform->SetWorldPosition(this->arrowPosition);

    ModelComponent* model = arrowEnt->AddComponent<ModelComponent>();
    model->SetModel(this->arrowModel);

    BoxCollisionComponent* boxCollision = arrowEnt->AddComponent<BoxCollisionComponent>();
    boxCollision->SetExtents({ 0.1f,0.2f,0.2f });
    boxCollision->SetLayer(CollisionLayer::PROJECTILE);
    boxCollision->SetActive(true);
    boxCollision->SetStatic(false);
    boxCollision->SetTrigger(true);

    HitboxComponent* hitbox = arrowEnt->AddComponent<HitboxComponent>();
    hitbox->SetKnockbackForce(3.0f);
    hitbox->ActivateForEver(true);
    hitbox->SetDamage(this->arrowDamage);

    ProjectileComponent* arrowProj = arrowEnt->AddComponent<ProjectileComponent>();
    arrowProj->SetGravity(this->arrowGravity);
    arrowProj->SetRestitution(0.0f);

    // Get player future position to shoot an arrow there
    XMFLOAT3 arrowStartPosF = this->arrowPosition;
    XMVECTOR arrowStartPosV = XMLoadFloat3(&arrowStartPosF);

    XMFLOAT3 playerPositionF = playerEnt->GetComponent<TransformComponent>()->GetWorldPosition();
    XMVECTOR playerPositionV = XMLoadFloat3(&playerPositionF);

    XMFLOAT3 playerVelocityF = playerEnt->GetComponent<PlayerControllerComponent>()->GetVelocity();
    XMVECTOR playerVelocityV = XMLoadFloat3(&playerVelocityF);

    float timeToTarget = 1.0f;

    XMVECTOR futurePositionV = XMVectorAdd(playerPositionV, XMVectorScale(playerVelocityV, timeToTarget));

    XMVECTOR displacementV = XMVectorSubtract(futurePositionV, arrowStartPosV);
    XMFLOAT3 displacement;
    XMStoreFloat3(&displacement, displacementV);

    XMFLOAT3 launchVelocity;

    launchVelocity.x = displacement.x / timeToTarget;
    launchVelocity.z = displacement.z / timeToTarget;

    float g = this->arrowGravity;
    launchVelocity.y = (displacement.y / timeToTarget) + (0.5f * g * timeToTarget);

    arrowProj->Launch(launchVelocity, { 0.0f, 0.0f, 0.0f });
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
    if (soundComponent) {
        if (enemyHealth && enemyHealth->IsDead()) {
            if (this->deathSound) {
                soundComponent->SetSound(this->deathSound);
                soundComponent->Play();
            }
        }
        else if (this->hitSound) {
            soundComponent->SetSound(this->hitSound);
            soundComponent->Play();
        }
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

    if (this->randomSound)
    {
        this->randomSoundTimer -= deltaTime;
        if (this->randomSoundTimer <= 0.0f)
        {
            this->randomSoundTimer = RNG::Float(this->randomSoundMinInterval, this->randomSoundMaxInterval);
            if (auto* soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
            {
                soundComponent->SetSound(this->randomSound);
                soundComponent->Play();
            }
        }
    }

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

    if (!this->animator)
    {
        this->animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    }

    XMFLOAT3 playerPosF = playerT->GetWorldPosition();
    XMVECTOR player = XMLoadFloat3(&playerPosF);

    XMVECTOR toPlayer = XMVectorSetY(XMVectorSubtract(player, myPos), 0.0f);
    const float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));
    const bool inAttackRange = distSq <= (this->stopDistance * this->stopDistance);

    if (distSq > 1e-4f)
    {
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, toPlayer);
        const float angleDeg = atan2f(dir.x, dir.z) * (180.0f / XM_PI);
        this->transform->SetWorldRotation(0.0f, angleDeg, 0.0f);
    }

    EnemyStats stats = GetEnemyStats(this->enemyType);

    if (this->isAttacking)
    {
        if (this->enemyType == EnemyType::Skeleton && inAttackRange && this->knockbackStunTimer <= 0.0f)
        {
            static float shootTimer = 0.0f;
            shootTimer -= deltaTime;
            if (shootTimer <= 0.0f)
            {
                this->arrowPosition = this->transform->GetWorldPosition();
                this->ShootAnArrow(this->arrowPosition);
                shootTimer = this->cooldown;
            }
        }

        this->attackElapsed += deltaTime;

        if (!this->hitboxActivated && this->attackElapsed >= this->attackHitboxDelay)
        {
            this->hitboxActivated = true;
            if (this->hitbox)
                this->hitbox->Activate(this->attackHitboxDuration);
        }

        if (!this->animator || this->animator->IsFinished())
        {
            if (inAttackRange && this->knockbackStunTimer <= 0.0f && this->animator && this->animator->HasAnimation(stats.attackAnimation))
            {
                // restart the swing
                this->attackElapsed = 0.0f;
                this->hitboxActivated = false;
                this->animator->CrossFade(stats.attackAnimation, 0.1f, false);
            }
            else
            {
                this->isAttacking = false;
                this->attackElapsed = 0.0f;
                this->hitboxActivated = false;
                if (this->hitbox)
                    this->hitbox->Deactivate();

                if (this->animator && this->animator->HasAnimation(stats.walkAnimation))
                    this->animator->CrossFade(stats.walkAnimation, 0.15f, true);
            }
        }
    }
    else if (inAttackRange && this->knockbackStunTimer <= 0.0f)
    {
        this->isAttacking = true;
        this->attackElapsed = 0.0f;
        this->hitboxActivated = false;

        if (this->animator && this->animator->HasAnimation(stats.attackAnimation))
            this->animator->CrossFade(stats.attackAnimation, 0.1f, false);
    }

    XMVECTOR steer = XMVectorZero();

    if (!this->isAttacking && !inAttackRange)
    {
        XMVECTOR seek = XMVectorZero();
        if (distSq > 1e-4f && this->knockbackStunTimer <= 0.0f)
        {
            seek = XMVector3Normalize(toPlayer);
        }

        XMVECTOR avoid = XMVectorSetY(grid.ComputeAvoidance(myGridIndex,
            this->avoidanceRadius),
            0.0f);

        steer = XMVectorSetY(XMVectorAdd(seek, avoid), 0.0f);
    }

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

        if (this->footstepSound)
        {
            if (this->animator && (this->animator->IsPlaying("Walk") || this->animator->IsPlaying("Jog")))
            {
                const float currAnimTime = this->animator->GetTime();
                if (this->lastFootstepAnimTime >= 0.0f)
                {
                    const float prevAnimTime = this->lastFootstepAnimTime;
                    auto crossedTime = [prevAnimTime, currAnimTime](float targetTime) {
                        if (currAnimTime >= prevAnimTime)
                            return prevAnimTime < targetTime && currAnimTime >= targetTime;
                        return prevAnimTime < targetTime || currAnimTime >= targetTime;
                    };

                    if (crossedTime(0.1f) || crossedTime(0.85f))
                    {
                        if (auto* soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
                        {
                            soundComponent->SetSound(this->footstepSound);
                            soundComponent->Play();
                        }
                    }
                }
                this->lastFootstepAnimTime = currAnimTime;
            }
            else if (this->footstepIntervalMeters > 0.0f)
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
    }
    else
    {
        this->lastFootstepAnimTime = -1.0f;
    }

    if (steerLen > 1e-4f || hasKnockback)
    {
        myPos = XMVectorSetY(myPos, myPosF.y);

        XMFLOAT3 outPos;
        XMStoreFloat3(&outPos, myPos);
        this->transform->SetWorldPosition(outPos);
    }
}