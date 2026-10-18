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
    if (!this->hitSound && context.assetManager) {
        this->hitSound = context.assetManager->Get<Sound>("assets/sounds/goblin_hit_sound.sound");
    }
    if (!this->attackHitSound && context.assetManager && this->enemyType == EnemyType::Goblin) {
        this->attackHitSound = context.assetManager->Get<Sound>("assets/sounds/goblin_attack_hit.sound");
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

    this->GetOwner()->GetComponent<Health>()->GetOnDeath().Connect([this, context]() {

        XMFLOAT3 position = this->GetPosition();
        position.y += this->enemy->extends.y;

        this->gibs->SpawnGibs(
            position,
            RNG::Int(3, 6),
            GibType::GOBLIN
        );

        this->gibs->SpawnBlood(
            context.sceneManager->GetCurrentScene(), 
            position,
            1.0f
        );
    });

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

    this->audioManager = context.audioManager;
    if (this->enemyType == EnemyType::Skeleton && context.assetManager) {
        this->bowDrawClip = context.assetManager->Get<AudioClip>("assets/audio/sfx/Skeleton/bow_draw.ogg");
        this->bowFireClip = context.assetManager->Get<AudioClip>("assets/audio/sfx/Skeleton/bow_fire.ogg");
    }
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

void EnemyControllerComponent::SetEnemyType(EnemyTypeOld type)
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
    if (this->hitbox && event.attacker == this->hitbox->GetOwner())
    {
        if (this->attackHitSound)
        {
            if (auto* soundComponent = this->GetOwner()->GetComponent<SoundComponent>())
            {
                soundComponent->SetSound(this->attackHitSound);
                soundComponent->Play();
            }
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
    const float dist = sqrtf(distSq);

    const bool isRanged = (this->enemyType == EnemyType::Skeleton);
    const float meleeRange = this->stopDistance;

    // Zone flags
    bool tooClose = false;
    bool inShootBand = false;
    bool tooFar = false;

    if (isRanged)
    {
        tooClose = dist < this->minShootRadius;
        inShootBand = dist >= this->minShootRadius && dist <= this->maxShootRadius;
        tooFar = dist > this->maxShootRadius;
    }
    else
    {
        inShootBand = dist <= meleeRange;
    }

    if (distSq > 1e-4f)
    {
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, toPlayer);
        const float angleDeg = atan2f(dir.x, dir.z) * (180.0f / XM_PI);
        this->transform->SetWorldRotation(0.0f, angleDeg, 0.0f);
    }

    EnemyStats stats = GetEnemyStats(this->enemyType);

    // Attack 

    if (isRanged)
    {
        const bool canShoot = inShootBand
            && this->knockbackStunTimer <= 0.0f
            && !hasKnockback;

        if (this->knockbackStunTimer > 0.0f || hasKnockback)
        {
            this->isAttacking = false;
            this->attackElapsed = 0.0f;
            this->hitboxActivated = false;
        }

        if (this->isAttacking)
        {
            this->attackElapsed += deltaTime;

            if (!this->hitboxActivated && this->attackElapsed >= this->attackHitboxDelay)
            {
                this->hitboxActivated = true;
                this->ShootAnArrow();
                if (this->audioManager && this->bowFireClip && this->transform)
                {
                    this->audioManager->PlaySoundAt(
                        this->bowFireClip, false, this->transform->GetWorldPosition(), 1.0f, 1.0f, 1.0f, 25.0f);
                }
            }

            if (!this->animator || this->animator->IsFinished())
            {
                this->isAttacking = false;
                this->attackElapsed = 0.0f;
                this->hitboxActivated = false;
                this->shootTimer = this->cooldown;
            }
        }
        else if (canShoot)
        {
            this->shootTimer -= deltaTime;
            if (this->shootTimer <= 0.0f)
            {
                this->isAttacking = true;
                this->attackElapsed = 0.0f;
                this->hitboxActivated = false;

                if (this->animator && this->animator->HasAnimation(stats.attackAnimation))
                    this->animator->CrossFade(stats.attackAnimation, 0.1f, false);

                if (this->audioManager && this->bowDrawClip && this->transform)
                {
                    this->audioManager->PlaySoundAt(
                        this->bowDrawClip, false, this->transform->GetWorldPosition(), 2.0f, 1.0f, 25.0f, 25.0f);
                }
            }
        }
        else if (this->shootTimer > 0.0f)
        {
            this->shootTimer -= deltaTime;
        }

        if (this->hitbox) this->hitbox->Deactivate();
    }
    else
    {
        // melee logic
        if (this->isAttacking)
        {
            this->attackElapsed += deltaTime;

            if (!this->hitboxActivated && this->attackElapsed >= this->attackHitboxDelay)
            {
                this->hitboxActivated = true;
                if (this->hitbox)
                    this->hitbox->Activate(this->attackHitboxDuration);
            }

            if (!this->animator || this->animator->IsFinished())
            {
                if (inShootBand && this->knockbackStunTimer <= 0.0f
                    && this->animator && this->animator->HasAnimation(stats.attackAnimation))
                {
                    this->attackElapsed = 0.0f;
                    this->hitboxActivated = false;
                    this->animator->CrossFade(stats.attackAnimation, 0.1f, false);
                }
                else
                {
                    this->isAttacking = false;
                    this->attackElapsed = 0.0f;
                    this->hitboxActivated = false;
                    if (this->hitbox) this->hitbox->Deactivate();

                    if (this->animator && this->animator->HasAnimation(stats.walkAnimation))
                        this->animator->CrossFade(stats.walkAnimation, 0.15f, true);
                }
            }
        }
        else if (inShootBand && this->knockbackStunTimer <= 0.0f)
        {
            this->isAttacking = true;
            this->attackElapsed = 0.0f;
            this->hitboxActivated = false;

            if (this->animator && this->animator->HasAnimation(stats.attackAnimation))
                this->animator->CrossFade(stats.attackAnimation, 0.1f, false);
        }
    }

    // Steering

    XMVECTOR steer = XMVectorZero();
    bool shouldMove = false;

    if (this->knockbackStunTimer <= 0.0f)
    {
        if (isRanged)
        {
            if (!this->isAttacking)
            {
                if (tooClose && distSq > 1e-4f)
                {
                    XMVECTOR away = XMVector3Normalize(XMVectorNegate(toPlayer));
                    steer = away;
                    shouldMove = true;
                }
                else if (tooFar && distSq > 1e-4f)
                {
                    XMVECTOR seek = XMVector3Normalize(toPlayer);
                    steer = seek;
                    shouldMove = true;
                }
            }
        }
        else
        {
            // Melee
            if (!this->isAttacking && !inShootBand && distSq > 1e-4f)
            {
                steer = XMVector3Normalize(toPlayer);
                shouldMove = true;
            }
        }

        if (shouldMove)
        {
            // Add avoidance on top of seek/flee
            XMVECTOR avoid = XMVectorSetY(
                grid.ComputeAvoidance(myGridIndex, this->avoidanceRadius), 0.0f);

            steer = XMVectorSetY(XMVectorAdd(steer, avoid), 0.0f);
        }
    }


    // Apply movement
    const float steerLen = XMVectorGetX(XMVector3Length(steer));
    if (steerLen > 1e-4f)
    {
        float effectiveSteerLen = steerLen;
        if (steerLen > 1.0f)
        {
            steer = XMVectorScale(steer, 1.0f / steerLen);
            effectiveSteerLen = 1.0f;
        }

        float speedScale = 1.0f;
        if (isRanged && tooClose)
            speedScale = this->fleeSpeedMultiplier;

        const float speed = this->moveSpeed * speedScale;
        const float distMoved = effectiveSteerLen * speed * deltaTime;

        myPos = XMVectorAdd(myPos, XMVectorScale(steer, speed * deltaTime));

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