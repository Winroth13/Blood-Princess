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
#include "components/gameplay/splatter_component.hpp"
#include "assets/asset_manager.hpp"
#include "components/gameplay/health_component.hpp"
#include "components/skeletal/animator_component.hpp"
#include "core/rng.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/gameplay/gibs_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "scene/scene_manager.hpp"
#include "assets/types/enemy.hpp"
#include "components/core/pool_component.hpp"
#include "components/core/lifetime_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/core/model_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "components/lights/point_light_component.hpp"
#include "components/collisions/area_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
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
    this->splatter = this->GetOwner()->GetComponent<SplatterComponent>();
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
    this->smokeTexture = context.assetManager->Get<Texture2D>("assets/images/detailed_smoke_spritesheet.png");
    this->arrowPassbySound = context.assetManager->Get<Sound>("assets/sounds/arrow_passby.sound");

    // Some dumb assumptions are made here but it's fine.
    auto &children = this->GetOwner()->GetChildren();
    this->hitboxEntity = children[0];
}

void EnemyControllerComponent::Update(const AppContext &context, float deltaTime) {
    ModelComponent *modelComp = this->GetOwner()->GetComponent<ModelComponent>();

    modelComp->SetEmissive(0.0f);
    if (this->emissiveTimer > 0.0f) {
        this->emissiveTimer -= deltaTime;
        modelComp->SetEmissive(10.0f);
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

    SoundComponent *arrowSound = arrowEnt->AddComponent<SoundComponent>();
    arrowSound->SetSound(this->arrowPassbySound);
    arrowSound->SetIs3d(true);
    arrowSound->SetMaxDistance(12.0f);
    arrowSound->SetVolume(5.0f);

    // fire entity
    Entity* fireEnt = this->thisScene->AddEntity("flame");
    fireEnt->SetParent(arrowEnt);

    // we attach the audio to fire ent so that it can have it's own collision
    SphereCollisionComponent *sphereCol = fireEnt->AddComponent<SphereCollisionComponent>();
    sphereCol->SetRadius(8.0f);
    sphereCol->SetLayer(CollisionLayer::ENEMY_ATTACK);
    sphereCol->SetTrigger(true);

    AreaComponent *area = fireEnt->AddComponent<AreaComponent>();
    area->GetOnAreaEntered().Connect([area, arrowSound](CollisionComponent* other) {
        area->SetEnabled(false);
        arrowSound->Play();
    });

    PointLightComponent *pointLight = fireEnt->AddComponent<PointLightComponent>();
    pointLight->SetColour({0.88f, 0.34f, 0.13f});
    pointLight->SetIntensity(75.0f);
    pointLight->SetRadius(8.0f);

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
    smoke->SetAtlasSize({ 8.0f, 8.0f });
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

    smoke->SetStartTint({ 0.2f, 0.2f, 0.2f }); // Ljusgrå start
    smoke->SetEndTint({ 0.05f, 0.05f, 0.05f }); // Mörkgrå slut 

}

void EnemyControllerComponent::ShootAnFireball(float gravity, float fireSpeed, XMFLOAT3 offset, bool wall)
{
    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    if (!playerEnt) return;

    TransformComponent* playerTransform = playerEnt->GetComponent<TransformComponent>();
    PlayerControllerComponent* playerController = playerEnt->GetComponent<PlayerControllerComponent>();
    if (!playerTransform || !playerController) return;

    XMFLOAT3 fireStartPos;

    XMFLOAT3 wizardPos = this->transform->GetWorldPosition();
    XMVECTOR wizardPosV = XMLoadFloat3(&wizardPos);

    XMFLOAT3 playerPos = playerTransform->GetWorldPosition();
    XMVECTOR playerPosV = XMLoadFloat3(&playerPos);

    XMVECTOR toPlayer = XMVectorSubtract(playerPosV, wizardPosV);
    XMVECTOR toPlayerHorizontal = XMVectorSetY(toPlayer, 0.0f);
    XMVECTOR forwardDir = XMVector3Normalize(toPlayerHorizontal);

    XMVECTOR startPosV = XMVectorAdd(wizardPosV, XMLoadFloat3(&this->fireballOffset));
    startPosV = XMVectorAdd(startPosV, XMVectorScale(forwardDir, 0.6f));
    XMStoreFloat3(&fireStartPos, startPosV);

    // Predict future player position & lateral offset
    XMFLOAT3 playerVelocityF = playerController->GetVelocity();
    XMVECTOR playerVelocityV = XMLoadFloat3(&playerVelocityF);

    // worldUp is (0, 1, 0)
    XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR rightDir = XMVector3Cross(worldUp, forwardDir);

    XMVECTOR targetV = XMVectorAdd(
        playerPosV,
        XMVectorAdd(
            XMVectorScale(rightDir, offset.x),   // left / right
            XMVectorAdd(
                XMVectorSet(0.0f, offset.y == 0.0f ? 0.1f : offset.y, 0.0f, 0.0f),
                XMVectorScale(forwardDir, offset.z) // in front / behind
            )
        )
    );

    // Calculate timeToTarget & ballistics
    XMVECTOR diffV = XMVectorSubtract(targetV, startPosV);
    float distToPlayer = XMVectorGetX(XMVector3Length(diffV));

    float timeToTarget = distToPlayer / fireSpeed;
    float arcModifier = 1.5f;
    timeToTarget *= arcModifier;

    if (timeToTarget < 0.1f) timeToTarget = 0.1f;

    // Future position = current position + velocity * timeToTarget
    XMVECTOR futurePositionV = XMVectorAdd(targetV, XMVectorScale(playerVelocityV, timeToTarget));

    // Ballistic trajectory:
    // displacement = futurePosition - startPos
    XMVECTOR displacementV = XMVectorSubtract(futurePositionV, startPosV);
    XMFLOAT3 displacement;
    XMStoreFloat3(&displacement, displacementV);

    XMFLOAT3 launchVelocity;
    launchVelocity.x = displacement.x / timeToTarget;
    launchVelocity.z = displacement.z / timeToTarget;

    float g = gravity;
    launchVelocity.y = (displacement.y / timeToTarget) + (0.5f * g * timeToTarget);

    // Create fire entity
    Entity* fireballEnt = this->thisScene->AddEntity("Fireball");

    TransformComponent* fireballTransform = fireballEnt->AddComponent<TransformComponent>();
    fireballTransform->SetWorldPosition(fireStartPos);

    SphereCollisionComponent* sphereCollision = fireballEnt->AddComponent<SphereCollisionComponent>();
    sphereCollision->SetRadius(this->fireballRadius);
    sphereCollision->SetLayer(CollisionLayer::PROJECTILE);
    sphereCollision->SetActive(true);
    sphereCollision->SetStatic(false);
    sphereCollision->SetTrigger(true);

    HitboxComponent* hitbox = fireballEnt->AddComponent<HitboxComponent>();
    hitbox->SetKnockbackForce(3.0f);
    hitbox->SetDamage(this->fireballDamage);
    hitbox->ActivateForEver(true);
    hitbox->SetDestroyOnHit(true);
    hitbox->IgnoreEntity(this->GetOwner());

    ProjectileComponent* arrowProj = fireballEnt->AddComponent<ProjectileComponent>();
    arrowProj->SetGravity(gravity);
    arrowProj->SetRestitution(0.0f);
    arrowProj->SetGroundY(-2.5f);
    arrowProj->SetDestroyOnGrounded(true);
    arrowProj->SetAliveTime(10.0f);
    arrowProj->Launch(launchVelocity, { 0.0f, 0.0f, 0.0f });

    // Flame particle system
    ParticleSystemComponent* flame = fireballEnt->AddComponent<ParticleSystemComponent>();
    flame->SetAtlasTexture(this->flameTexture);
    flame->SetAtlasSize({ 12.0f, 6.0f });
    flame->SetVelocity({ 0.0f, 0.0f, 0.0f });
    flame->SetAcceleration({ 0.0f, 0.0f, 0.0f });
    flame->SetLifeTime(timeToTarget*2.5);
    flame->SetIsAnimate(true);
    flame->SetISFlat(false);
    flame->SetIsAdditive(true);
    flame->SetAnimationSpeed(1.2f);
    flame->SetCount(this->fireballRadius * 100);
    flame->SetSpawnRadius(this->fireballRadius);
    if (wall) {
        flame->SetIsSpread(true);
        flame->SetStartScale(0.8f);
        flame->SetEndScale(0.8f);
    }
    else
    {
        flame->SetStartScale(0.4f);
        flame->SetEndScale(0.4f);
    }

    // Smoke particle system
    ParticleSystemComponent* smoke = fireballEnt->AddComponent<ParticleSystemComponent>();
    smoke->SetAtlasTexture(this->smokeTexture);
    smoke->SetAtlasSize({ 8.0f, 8.0f });
    smoke->SetVelocity({ 0.0f, 0.5f, 0.0f });
    smoke->SetAcceleration({ 0.0f, 0.0f, 0.0f });
    smoke->SetLifeTime(timeToTarget * 2.5);
    smoke->SetStartScale(0.3f);
    smoke->SetEndScale(0.3f);
    smoke->SetIsAnimate(true);
    smoke->SetIsGlobal(true);
    smoke->SetISFlat(false);
    smoke->SetIsAdditive(false);
    smoke->SetSpawnRadius(this->fireballRadius);
    smoke->SetCount(this->fireballRadius * 100);

    smoke->SetStartTint({ 0.2f, 0.2f, 0.2f }); // Ljusgrå start
    smoke->SetEndTint({ 0.05f, 0.05f, 0.05f }); // Mörkgrå slut 

    SoundComponent* fireballSound = fireballEnt->AddComponent<SoundComponent>();
    fireballSound->SetSound(this->arrowPassbySound);
    fireballSound->SetIs3d(true);
    fireballSound->SetMaxDistance(12.0f);
    fireballSound->SetVolume(5.0f);

    // fire entity
    Entity* fireEnt = this->thisScene->AddEntity("flame");
    fireEnt->SetParent(fireballEnt);

    fireEnt->AddComponent<TransformComponent>()->SetWorldPosition(fireballTransform->GetWorldPosition());
    // we attach the audio to fire ent so that it can have it's own collision
    SphereCollisionComponent* sphereCol = fireEnt->AddComponent<SphereCollisionComponent>();
    sphereCol->SetRadius(8.0f);
    sphereCol->SetLayer(CollisionLayer::ENEMY_ATTACK);
    sphereCol->SetTrigger(true);

    AreaComponent* area = fireEnt->AddComponent<AreaComponent>();
    area->GetOnAreaEntered().Connect([area, fireballSound](CollisionComponent* other) {
        area->SetEnabled(false);
        fireballSound->Play();
        });

    /*
    PointLightComponent* pointLight = fireEnt->AddComponent<PointLightComponent>();
    pointLight->SetColour({ 0.88f, 0.34f, 0.13f });
    pointLight->SetIntensity(75.0f);
    pointLight->SetRadius(8.0f);
    */
}

void EnemyControllerComponent::ShootVerticalRowOfFireball()
{
    for (int i = -2; i < 6; i++)
    {
        this->ShootAnFireball(this->fireballHorizontalGravity, this->fireballHorizontalSpeed, XMFLOAT3(0.0f, (float)i / 2.0f, 0));
    }
}

void EnemyControllerComponent::ShootHorizontalRowOfFireball()
{
    for (int i = -3; i < 4; i++)
    {
        this->ShootAnFireball(this->fireballVerticalGravity, this->fireballVerticalSpeed, XMFLOAT3((float)i, 0, 0));
    }
}

void EnemyControllerComponent::ShootCircleOfFireball()
{
    const float angleStep = DirectX::XM_2PI / 8.0f; // 8 fireball

    const int innerCount = 4;
    const float innerRadius = 0.3f;
    const float innerAngleStep = DirectX::XM_2PI / static_cast<float>(innerCount);
    for (int i = 0; i < innerCount; ++i)
    {
        float angle = static_cast<float>(i) * innerAngleStep;
        float offsetX = cosf(angle) * innerRadius;
        float offsetZ = sinf(angle) * innerRadius;

        this->ShootAnFireball(
            this->fireballVerticalGravity,
            this->fireballVerticalSpeed,
            DirectX::XMFLOAT3(offsetX, 0.0f, offsetZ),
            true
        );
    }


    for (int i = 0; i < 8; ++i)
    {
        float angle = static_cast<float>(i) * angleStep;
        float radius = 0.7f;

        // Position fireballs along the circle circumference
        float offsetX = radius * cosf(angle);
        float offsetY = radius * sinf(angle);

        this->ShootAnFireball(
            this->fireballVerticalGravity,
            this->fireballVerticalSpeed,
            DirectX::XMFLOAT3(offsetX, offsetY, 0),
            true
        );
    }
}

void EnemyControllerComponent::ShootRainOfFireball()
{
    // 1. Center fireball (directly at player)
    this->ShootAnFireball(
        13.0,
        15.30,
        DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
        true
    );

    // 2. Inner ring of 6 fireballs filling the middle area
    const int innerCount = 6;
    const float innerRadius = 1.8f;
    const float innerAngleStep = DirectX::XM_2PI / static_cast<float>(innerCount);
    for (int i = 0; i < innerCount; ++i)
    {
        float angle = static_cast<float>(i) * innerAngleStep;
        float offsetX = cosf(angle) * innerRadius;
        float offsetZ = sinf(angle) * innerRadius;

        this->ShootAnFireball(
            13.0,
            15.30,
            DirectX::XMFLOAT3(offsetX, 0.0f, offsetZ),
            true
        );
    }

    // 3. Outer ring of 12 fireballs landing around the perimeter
    const int outerCount = 12;
    const float outerRadius = 3.5f;
    const float outerAngleStep = DirectX::XM_2PI / static_cast<float>(outerCount);
    for (int i = 0; i < outerCount; ++i)
    {
        float angle = static_cast<float>(i) * outerAngleStep;
        float offsetX = cosf(angle) * outerRadius;
        float offsetZ = sinf(angle) * outerRadius;

        this->ShootAnFireball(
            13.0,
            15.30,
            DirectX::XMFLOAT3(offsetX, 0.0f, offsetZ),
            true
        );
    }
}

void EnemyControllerComponent::ShootWallOfFireball()
{
    for (int x = -3; x < 4; x++)
    {
        for (int y = -3; y < 4; y++)
        {
            this->ShootAnFireball(this->fireballVerticalGravity, this->fireballVerticalSpeed, XMFLOAT3((float)x, (float)y / 2.0f, 0), true);
        }
    }
}

void EnemyControllerComponent::WizardFireballAttack()
{
    auto healthComp = this->GetOwner()->GetComponent<Health>();
    float healthPct = 1.0f;
    if (healthComp && healthComp->GetMaxHealth() > 0.0f)
    {
        healthPct = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
    }

    if (healthPct > 0.75f)
    {
        // Phase 1 (HP > 75%): Default attack
        this->fireballBurstRemaining = this->fireballBurstCount;
        this->fireballBurstTimer = 0.0f;
    }
    else if (healthPct > 0.50f)
    {
        // Phase 2 (50% < HP <= 75%): Horizontal row of fireballs
        this->ShootHorizontalRowOfFireball();
    }
    else if (healthPct > 0.25f)
    {
        // Phase 3 (25% < HP <= 50%): Vertical column of fireballs
        this->ShootVerticalRowOfFireball();
    }
    else
    {
        // Phase 4 (HP <= 25%): Desperation attack - Rain of fireballs parabolic artillery ring
        this->ShootRainOfFireball();
    }
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

    XMFLOAT3 position = this->GetPosition();
    switch (this->enemy->type)
    {
    case EnemyType::GOBLIN:
    {
        this->splatter->PaintFloorAtPosition(position, RNG::Float(this->hitBloodRadiusMin, this->hitBloodRadiusMax));
    } break;
    case EnemyType::OGRE:
    {
        this->splatter->PaintFloorAtPosition(position, RNG::Float(this->hitBloodRadiusMin, this->hitBloodRadiusMax) * this->ogreBloodRediousMult);
    } break;

    case EnemyType::SKELETON:
        break;
    }

    this->emissiveTimer = this->emissiveDuration;
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
    this->GetOwner()->GetComponent<ModelComponent>()->SetEmissive(0.0f);
    this->emissiveTimer = 0.0f;

    XMFLOAT3 position = this->GetPosition();
    XMFLOAT3 feetPosition = position;
    position.y += this->enemy->extends.y;

    switch (this->enemy->type)
    {
        case EnemyType::GOBLIN:
        {
            this->splatter->PaintFloorAtPosition(feetPosition, RNG::Float(this->deathBloodRadiusMin, this->deathBloodRadiusMax));
            this->gibs->SpawnGibs(position, RNG::Int(3, 6), GibType::GOBLIN);
            this->gibs->SpawnBlood(context.sceneManager->GetCurrentScene(), position, 1.0f);
        } break;
        case EnemyType::OGRE:
        {
            this->splatter->PaintFloorAtPosition(feetPosition, RNG::Float(this->deathBloodRadiusMin, this->deathBloodRadiusMax) * this->ogreBloodRediousMult);
            this->gibs->SpawnGibs(position, RNG::Int(3, 6), GibType::OGRE);
            this->gibs->SpawnBlood(context.sceneManager->GetCurrentScene(), position,1.0f);
        } break;
        case EnemyType::SKELETON:
        {
            this->gibs->SpawnGibs(position, RNG::Int(3, 6), GibType::SKELETON);
            this->gibs->SpawnBlood(context.sceneManager->GetCurrentScene(), position, 1.0f);
        } break;
        case EnemyType::WIZARD:
        {
            this->gibs->SpawnBlood(context.sceneManager->GetCurrentScene(), position, 10.0f);
        }break;
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
    this->fireballBurstRemaining = 0;
    this->fireballBurstTimer = 0.0f;

    auto *animator = this->GetOwner()->GetComponent<AnimatorComponent>();
    this->animator->CrossFade(this->enemy->attackAnimation, 0.1f, false);
    this->animator->SetSpeed(1.0f);

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
    this->fireballBurstRemaining = 0;
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
    if (enemy->type == EnemyType::WIZARD || this->fireballBurstRemaining > 0)
    {
        this->fireballBurstTimer -= deltaTime;
        if (this->fireballBurstTimer <= 0.0f)
        {
            this->ShootAnFireball(this->fireballGravity, this->fireballSpeed, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
            this->fireballBurstRemaining--;
            this->fireballBurstTimer = this->fireballBurstInterval;
            if (this->enemy->attackSound)
            {
                this->sound->SetSound(this->enemy->attackSound);
                this->sound->Play();
            }
        }
    }

    if (this->animator->IsFinished() && this->fireballBurstRemaining <= 0)
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
            this->MeleeAttack(5.0, 0.5f, {0.0f, 1.02f, 0.65f}, {0.2f, 0.5f, 0.4f});
        } break;
        case EnemyType::OGRE:
        {
            this->MeleeAttack(50, 0.5f, {0.0f, 0.73f, 1.45f}, {0.5f, 0.5f, 0.5f});

        } break;
        case EnemyType::SKELETON:
        {
            this->FireProjectile();
        } break;
        case EnemyType::WIZARD:
        {
            this->WizardFireballAttack();
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
    ShootAnArrow();
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