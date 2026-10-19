#include "enemy_controller_component.hpp"
#include "components/core/transform_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
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
#include "components/gameplay/gibs_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "scene/scene_manager.hpp"
#include "assets/types/enemy.hpp"

#include "components/gameplay/projectile_component.hpp"
#include "components/core/model_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "audio/audio_manager.hpp"
#include "assets/types/audio_clip.hpp"
#include <algorithm>
#include <DirectXMath.h>

uint32_t s_arrowCounter = 0;

void EnemyControllerComponent::OnStart(const AppContext& context)
{
    this->randomSoundTimer = RNG::Float(0.0f, 15.0f);

    this->transform = this->GetOwner()->GetComponent<TransformComponent>();
    this->animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    this->sound = this->GetOwner()->GetComponent<SoundComponent>();
    this->lastFootstepAnimTime = this->animator ? this->animator->GetTime() : -1.0f;

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

    this->GetOwner()->GetComponent<Health>()->GetOnDeath().Connect([this, context]() {
        this->OnDeath(context);
    });

    // TODO: Fix later so that this doesn't spawn blood when the enemy is healed (respawned)
    this->GetOwner()->GetComponent<Health>()->GetOnChangedHealth().Connect([this, context](float currentHealth, float maxHealth) {

        XMFLOAT3 position = this->GetPosition();
        position.y += this->enemy->extends.y;

        this->gibs->SpawnBlood(
            context.sceneManager->GetCurrentScene(),
            position,
            0.4f
        );
    });

    this->GetOwner()->GetScene()->AddEntityToGroup(this->groupName, this->GetOwner());

    this->thisScene = this->GetOwner()->GetScene();

    this->arrowModel = context.assetManager->Get<Model>("assets/models/arrow.glb");
    this->flameTexture = context.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png");
    this->smokeTexture = context.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png");

    // Some dumb assumptions are made here but it's fine.
    auto &children = this->GetOwner()->GetChildren();
    this->hitboxEntity = children[0];
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
    const float totalForce = force * this->enemy->knockbackFactor;

    if (totalForce <= 0.0f)
    {
        return;
    }

    this->knockbackVelocity.x = direction.x * totalForce;
    this->knockbackVelocity.y = 0.0f;
    this->knockbackVelocity.z = direction.z * totalForce;
    this->knockbackStunTimer = this->enemy->knockbackStunDuration;
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

    auto *hitbox = this->hitboxEntity->GetComponent<HitboxComponent>();

    if (hitbox)
        hitbox->Deactivate();
}

void EnemyControllerComponent::ShootAnArrow()
{
    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    if (!playerEnt) return;

    TransformComponent* playerTransform = playerEnt->GetComponent<TransformComponent>();
    PlayerControllerComponent* playerController = playerEnt->GetComponent<PlayerControllerComponent>();
    if (!playerTransform || !playerController) return;

    XMFLOAT3 arrowStartPos;

    XMFLOAT3 skelPos = this->transform->GetWorldPosition();
    XMVECTOR skelPosV = XMLoadFloat3(&skelPos);

    XMFLOAT3 playerPos = playerTransform->GetWorldPosition();
    XMVECTOR playerPosV = XMLoadFloat3(&playerPos);

    XMVECTOR toPlayer = XMVectorSubtract(playerPosV, skelPosV);
    XMVECTOR toPlayerHorizontal = XMVectorSetY(toPlayer, 0.0f);
    XMVECTOR forwardDir = XMVector3Normalize(toPlayerHorizontal);

    XMVECTOR startPosV = XMVectorAdd(skelPosV, XMLoadFloat3(&this->arrowOffset));
    startPosV = XMVectorAdd(startPosV, XMVectorScale(forwardDir, 0.6f));
    XMStoreFloat3(&arrowStartPos, startPosV);

    XMVECTOR arrowStartPosV = XMLoadFloat3(&arrowStartPos);

    // Predict future player position
    XMFLOAT3 playerPositionF = playerTransform->GetWorldPosition();
    playerPositionF.y += 0.1f; // Aim at player's center of mass (playerPos.y + 0.3f)
    
    XMFLOAT3 playerVelocityF = playerController->GetVelocity();
    XMVECTOR playerVelocityV = XMLoadFloat3(&playerVelocityF);

    XMVECTOR vectorZero = XMVectorZero();
    if (!XMVector3Equal(playerVelocityV, vectorZero)) {

        playerPositionF.x += RNG::Float(-0.1f, 0.1f);
        playerPositionF.z += RNG::Float(-0.1f, 0.1f);
    }

    XMVECTOR playerPositionV = XMLoadFloat3(&playerPositionF);
    // Calculate timeToTarget
    XMVECTOR diffV = XMVectorSubtract(playerPositionV, arrowStartPosV);
    float distToPlayer = XMVectorGetX(XMVector3Length(diffV));
    float speed = this->arrowSpeed > 1.0f ? this->arrowSpeed : 20.0f;

    float timeToTarget = distToPlayer / speed;
    float arcModifier = 1.5f;
    timeToTarget *= arcModifier;

    if (timeToTarget < 0.1f) timeToTarget = 0.1f;

    // Future position = current position + velocity * timeToTarget
    XMVECTOR futurePositionV = XMVectorAdd(playerPositionV, XMVectorScale(playerVelocityV, timeToTarget));


    // Ballistic trajectory:
    // displacement = futurePosition - arrowStartPos
    XMVECTOR displacementV = XMVectorSubtract(futurePositionV, arrowStartPosV);
    XMFLOAT3 displacement;
    XMStoreFloat3(&displacement, displacementV);

    XMFLOAT3 launchVelocity;
    launchVelocity.x = displacement.x / timeToTarget;
    launchVelocity.z = displacement.z / timeToTarget;

    float g = this->arrowGravity;
    launchVelocity.y = (displacement.y / timeToTarget) + (0.5f * g * timeToTarget);

    // Create arrow entity
    Entity* arrowEnt = this->thisScene->AddEntity("Arrow");

    TransformComponent* transform = arrowEnt->AddComponent<TransformComponent>();
    transform->SetWorldPosition(arrowStartPos);

    // Initial rotation facing launch direction
    DirectX::XMVECTOR launchVelVec = DirectX::XMLoadFloat3(&launchVelocity);
    DirectX::XMVECTOR f = DirectX::XMVector3Normalize(launchVelVec);

    DirectX::XMVECTOR row0 = DirectX::XMVectorNegate(f);

    DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    if (std::abs(DirectX::XMVectorGetY(f)) > 0.99f)
    {
        worldUp = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    DirectX::XMVECTOR row2 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(row0, worldUp));
    DirectX::XMVECTOR row1 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(row2, row0));

    DirectX::XMMATRIX rotMat;
    rotMat.r[0] = row0;
    rotMat.r[1] = row1;
    rotMat.r[2] = row2;
    rotMat.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationMatrix(rotMat);
    DirectX::XMFLOAT4 quat;
    DirectX::XMStoreFloat4(&quat, q);
    transform->SetWorldRotationQuaternion(quat);
    // end rotation

    ModelComponent* model = arrowEnt->AddComponent<ModelComponent>();
    model->SetModel(this->arrowModel);

    BoxCollisionComponent* boxCollision = arrowEnt->AddComponent<BoxCollisionComponent>();
    boxCollision->SetExtents({ 0.5f, 0.25f, 0.25f });
    boxCollision->SetLayer(CollisionLayer::PROJECTILE);
    boxCollision->SetActive(true);
    boxCollision->SetStatic(false);
    boxCollision->SetTrigger(true);

    HitboxComponent* hitbox = arrowEnt->AddComponent<HitboxComponent>();
    hitbox->SetKnockbackForce(3.0f);
    hitbox->SetDamage(this->arrowDamage);
    hitbox->ActivateForEver(true);
    hitbox->SetDestroyOnHit(true);
    hitbox->IgnoreEntity(this->GetOwner());

    ProjectileComponent* arrowProj = arrowEnt->AddComponent<ProjectileComponent>();
    arrowProj->SetGravity(this->arrowGravity);
    arrowProj->SetRestitution(0.0f);
    arrowProj->SetGroundY(-2.5f);
    arrowProj->SetDestroyOnGrounded(true);
    arrowProj->SetAliveTime(50.0f);
    arrowProj->Launch(launchVelocity, { 0.0f, 0.0f, 0.0f });

    // fire entity
    Entity* fireEnt = this->thisScene->AddEntity("flame");
    fireEnt->SetParent(arrowEnt);


    TransformComponent* fireTransform = fireEnt->AddComponent<TransformComponent>();
    XMFLOAT3 arrowLengthOffset = {-0.25f, 0.1f, 0.0f};

    fireTransform->SetLocalPosition(arrowLengthOffset);
    fireTransform->SetLocalRotationQuaternion({ 0.0f, 0.0f, 0.0f, 1.0f });

    // Flame particle system
    ParticleSystemComponent* flame = fireEnt->AddComponent<ParticleSystemComponent>();
    flame->SetAtlasTexture(this->flameTexture);
    flame->SetAtlasSize({ 12.0f, 6.0f });
    flame->SetVelocity({ 0.0f, 0.0f, 0.0f });
    flame->SetAcceleration({ 0.0f, 0.0f, 0.0f });
    flame->SetLifeTime(timeToTarget+50);
    flame->SetStartScale(0.4f);
    flame->SetEndScale(0.4f);
    flame->SetIsAnimate(true);
    flame->SetIsAdditive(true);
    flame->SetSpawnRadius(0.0f);
    flame->SetAnimationSpeed(1.2f);
    flame->SetCount(3);

    // Smoke particle system
    ParticleSystemComponent* smoke = fireEnt->AddComponent<ParticleSystemComponent>();
    smoke->SetAtlasTexture(this->smokeTexture);
    smoke->SetAtlasSize({ 13.0f, 1.0f });
    smoke->SetVelocity({ 0.0f, 0.4f, 0.0f });
    smoke->SetAcceleration({ 0.0f, 0.0f, 0.0f });
    smoke->SetLifeTime(timeToTarget);
    smoke->SetStartScale(0.1f);
    smoke->SetEndScale(0.6f);
    smoke->SetIsAnimate(true);
    smoke->SetIsGlobal(true);
    smoke->SetIsAdditive(false);
    smoke->SetSpawnRadius(0.05f);
    smoke->SetCount(25);

    smoke->SetStartTint({ 0.6f, 0.6f, 0.6f }); // Ljusgrå start
    smoke->SetEndTint({ 0.2f, 0.2f, 0.2f }); // Mörkgrå slut 

}

void EnemyControllerComponent::OnAttackHit(const AttackHitEvent& event)
{
    if (event.attacker == this->hitboxEntity)
    {
        if (this->enemy->hitSound)
        {
            sound->SetSound(this->enemy->hitSound);
            sound->Play();
        }
        return;
    }

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

void EnemyControllerComponent::Move(float deltaTime)
{
    XMVECTOR velocityV = XMLoadFloat3(&this->velocity);
    XMVECTOR knockbackVelocityV = XMLoadFloat3(&this->knockbackVelocity);

    XMFLOAT3 position = this->GetPosition();
    XMVECTOR positionV = XMLoadFloat3(&position);

    XMVECTOR newPositionV = XMVectorAdd(positionV, velocityV * deltaTime);
    newPositionV = XMVectorAdd(newPositionV, knockbackVelocityV * deltaTime);

    XMFLOAT3 newPosition = {};
    XMStoreFloat3(&newPosition, newPositionV);

    this->transform->SetWorldPosition(newPosition);
}

void EnemyControllerComponent::OnDeath(const AppContext &context)
{
    XMFLOAT3 position = this->GetPosition();
    position.y += this->enemy->extends.y;

    switch (this->enemy->type)
    {
        case EnemyType::GOBLIN:
        case EnemyType::OGRE:
        {
            this->gibs->SpawnGibs(position, RNG::Int(3, 6), GibType::GOBLIN);
            this->gibs->SpawnBlood(context.sceneManager->GetCurrentScene(), position,1.0f);
        } break;

        case EnemyType::SKELETON:
            break;
    }
}

bool EnemyControllerComponent::CanAttack(float distanceSq)
{
    if (isAttacking)
         return false;

    const float kbSpeedSq = (this->knockbackVelocity.x * this->knockbackVelocity.x) +
        (this->knockbackVelocity.z * this->knockbackVelocity.z);

    const bool hasKnockback = kbSpeedSq > 0.001f;

    if (this->knockbackStunTimer > 0.0f || hasKnockback)
    {
        return false;
    }

    float minRangeSq = this->enemy->minRange * this->enemy->minRange;
    float maxRangeSq = this->enemy->maxRange * this->enemy->maxRange;

    if (!(distanceSq >= minRangeSq && distanceSq <= maxRangeSq))
    {
        return false;
    }

    return true;
}

void EnemyControllerComponent::StartAttack(float deltaTime)
{
    this->isAttacking = true;
    this->hasAttacked = false;
    this->attackElapsed = 0.0f;

    auto *animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    this->animator->CrossFade(this->enemy->attackAnimation, 0.1f, false);

    if (this->enemy->type == EnemyType::SKELETON)
    {
        this->sound->SetMinDistance(25.0f);
        this->sound->SetMaxDistance(25.0f);
    }

    this->sound->SetSound(this->enemy->startAttackSound);
    this->sound->Play();

    this->sound->SetMinDistance(1.0f);
    this->sound->SetMaxDistance(25.0f);
}

void EnemyControllerComponent::EndAttack()
{
    this->isAttacking = false;
    this->animator->CrossFade(this->enemy->walkAnimation, 0.1f);
}

void EnemyControllerComponent::UpdateAttack(float deltaTime)
{
    if (!this->isAttacking) 
        return;

    this->attackElapsed += deltaTime;

    if (!hasAttacked && attackElapsed >= this->enemy->attackTime)
    {
        this->Attack();
    }

    if (this->animator->IsFinished())
    {
        this->EndAttack();
    }
}

void EnemyControllerComponent::Attack()
{
    this->hasAttacked = true;

    this->sound->SetSound(this->enemy->attackSound);
    this->sound->Play();

    switch (this->enemy->type)
    {
        case EnemyType::GOBLIN:
        {
            this->MeleeAttack(5.0, 0.5f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f});
        } break;
        case EnemyType::OGRE:
        {
            this->MeleeAttack(50, 0.5f, {0.0f, 0.73f, 1.45f}, {0.5f, 0.5f, 0.5f});

        } break;
        case EnemyType::SKELETON:
        {
            this->FireProjectile();
        } break;
    }
}

void EnemyControllerComponent::MeleeAttack(float damage, float duration, XMFLOAT3 hitPosition, XMFLOAT3 hitExtends)
{
    auto hitbox = this->hitboxEntity->GetComponent<HitboxComponent>();
    auto transform = this->hitboxEntity->GetComponent<TransformComponent>();
    auto collision = this->hitboxEntity->GetComponent<BoxCollisionComponent>();

    transform->SetLocalPosition(hitPosition);

    collision->SetExtents(hitExtends);

    hitbox->SetDamage(damage);
    hitbox->Activate(duration);
}

void EnemyControllerComponent::FireProjectile()
{
    this->ShootAnArrow();
}

void EnemyControllerComponent::UpdateBark(float deltaTime)
{
    if (this->enemy->barkSound)
    {
        this->randomSoundTimer -= deltaTime;
        if (this->randomSoundTimer <= 0.0f)
        {
            this->randomSoundTimer = RNG::Float(
                this->randomSoundMinInterval, 
                this->randomSoundMaxInterval
            );

            this->sound->SetSound(this->enemy->barkSound);
            this->sound->Play();
        }
    }
}

DirectX::XMFLOAT3 EnemyControllerComponent::GetPosition() const
{
    if (!this->transform)
        return { 0.0f, 0.0f, 0.0f };

    return this->transform->GetWorldPosition();
}

void EnemyControllerComponent::UpdateAI(float deltaTime, size_t myGridIndex, const SpatialGrid& grid)
{
    if (!this->transform) 
        return;

    this->UpdateBark(deltaTime);

    XMFLOAT3 myPosF = this->transform->GetWorldPosition();
    XMVECTOR positionV = XMLoadFloat3(&myPosF);

    XMVECTOR kbVel = XMLoadFloat3(&this->knockbackVelocity);
    const float kbSpeedSq = XMVectorGetX(XMVector3LengthSq(kbVel));
    const bool hasKnockback = kbSpeedSq > 0.001f;

    if (this->knockbackStunTimer > 0.0f)
    {
        this->knockbackStunTimer -= deltaTime;
    }

    if (hasKnockback)
    {
        positionV = XMVectorAdd(positionV, XMVectorScale(kbVel, deltaTime));

        const float decay = (std::max)(0.0f, 1.0f - this->enemy->knockbackDecay * deltaTime);
        kbVel = XMVectorScale(kbVel, decay);
        XMStoreFloat3(&this->knockbackVelocity, kbVel);
    }

    Entity* playerEnt = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    TransformComponent* playerTransform = playerEnt->GetComponent<TransformComponent>();

    if (!this->animator)
    {
        this->animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    }

    XMFLOAT3 playerPosF = playerTransform->GetWorldPosition();
    XMVECTOR playerPos = XMLoadFloat3(&playerPosF);
    XMVECTOR toPlayerV = XMVectorSetY(XMVectorSubtract(playerPos, positionV), 0.0f);

    const float distSq = XMVectorGetX(XMVector3LengthSq(toPlayerV));
    
    XMVECTOR velocityV = {0.0f, 0.0f, 0.0f, 0.0f};

    if (this->CanAttack(distSq))
    {
        this->StartAttack(deltaTime);
    }
    else
    {
        if (!this->isAttacking)
        {
            // enemy should always move towards the player
            velocityV = XMVector3Normalize(toPlayerV) * this->enemy->moveSpeed;

            if (
                this->enemy->type == EnemyType::SKELETON 
                && distSq < this->enemy->minRange * this->enemy->minRange
            )
            {
                velocityV = -velocityV * 0.75; // enemies should flee
            }

            // enemy should avoid other enemies
            XMVECTOR avoidV = XMVectorSetY(grid.ComputeAvoidance(myGridIndex, this->enemy->avoidanceRadius), 0.0f);
            velocityV = XMVectorAdd(velocityV, avoidV);
        }

        // rotate enemy towards player
        if ((distSq > 1e-4f && !this->isAttacking) || enemy->type == EnemyType::SKELETON)
        {
            XMFLOAT3 dir;
            XMStoreFloat3(&dir, toPlayerV);
            const float angleDeg = atan2f(dir.x, dir.z) * (180.0f / XM_PI);
            this->transform->SetWorldRotation(0.0f, angleDeg, 0.0f);
        }
    }

    if (this->enemy->type != EnemyType::GOBLIN)
    {
        if (this->animator && !this->isAttacking)
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
                    if (auto *soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
                    {
                        soundComponent->SetSound(this->enemy->footstepSound);
                        soundComponent->Play();
                    }
                }
            }
            this->lastFootstepAnimTime = currAnimTime;
        }
    }
    
    XMStoreFloat3(&this->velocity, velocityV);
    this->Move(deltaTime);

    this->UpdateAttack(deltaTime);
}