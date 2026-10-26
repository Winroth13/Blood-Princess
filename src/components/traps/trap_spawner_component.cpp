#include "components/traps/trap_spawner_component.hpp"
#include "components/traps/trap_component.hpp"
#include "scene/scene.hpp"
#include "scene/entity.hpp"
#include "components/core/transform_component.hpp"
#include "components/core/model_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/model.hpp"
#include "assets/types/texture_2d.hpp"
#include "ai/ai_manager.hpp"
#include "core/rng.hpp"
#include "core/logging.hpp"
#include "components/skeletal/animator_component.hpp"
#include "components/audio/sound_component.hpp"
#include "assets/types/floor.hpp"


// helper function
bool TrapSpawnerComponent::SampleSpawnPosition(
    const DirectX::XMFLOAT3& origin,
    float innerR, float outerR,
    DirectX::XMFLOAT3& outPos)
{
    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");

    if (!playerEnt) return false;
    XMFLOAT3 playerPosF = playerEnt->GetComponent<TransformComponent>()->GetWorldPosition();
    XMVECTOR playerPosV = XMLoadFloat3(&playerPosF);

    const int maxAttempts = 30;
    for (int i = 0; i < maxAttempts; ++i)
    {
        float angle = RNG::Float(0.0f, DirectX::XM_2PI);

        float dist = RNG::Float(innerR, outerR);
        outPos = {
            origin.x + cosf(angle) * dist,
            origin.y,
            origin.z + sinf(angle) * dist
        };

        // look if the trap near the player
        XMFLOAT3 positionF = outPos;
        XMVECTOR positionV = XMLoadFloat3(&positionF);

        XMVECTOR distanceV = XMVectorSubtract(positionV, playerPosV);

        float distance = XMVectorGetX(XMVector3Length(distanceV));

        if (distance < 5.0f) {
            continue;
        }


        if (!this->aiManager || this->aiManager->GetGrid().IsAreaClearOfTraps(outPos, this->minTrapSpacing))
        {
            return true;
        }
    }
    return false;
}

void TrapSpawnerComponent::InitializeTrapPools(const AppContext& context, Scene* scene)
{
    this->fireTrapPool.reserve(this->maxFirePool);
    this->activeFireTraps.reserve(this->maxFirePool);

    Entity* fireRoot = scene->AddEntity("FireTraps");
    for (int i = 0; i < this->maxFirePool; ++i)
    {
        Entity* t = BuildFireTrap(scene, "Pooled_FireTrap_" + std::to_string(i));
        t->SetParent(fireRoot);
        this->fireTrapPool.push_back(t);
    }
}

Entity* TrapSpawnerComponent::BuildSpikeTrap(Scene * scene, const std::string & name, const SpikeTrapConfig& config)
{
    Entity* trap = scene->AddEntity(name);

    auto* transform = trap->AddComponent<TransformComponent>();
    transform->SetLocalScale({ 0.4f, 1.0f, 0.4f });

    auto* model = trap->AddComponent<ModelComponent>();
    model->SetModel(this->spikeTrapModel);

    auto* animator = trap->AddComponent<AnimatorComponent>();
    if (this->spikeTrapModel && this->spikeTrapModel->GetAnimations())
    {
        animator->AddAnimations(this->spikeTrapModel->GetAnimations());
    }

    auto* boxCollider = trap->AddComponent<BoxCollisionComponent>();
    boxCollider->SetLayer(CollisionLayer::TRIGGER_HAZARD);
    boxCollider->SetTrigger(true);
    boxCollider->SetStatic(false);
    boxCollider->SetExtents({ 5.0f, 2.5f, 5.0f });
    boxCollider->SetCenterOffset({ 0.0f, 2.0f, 0.0f });

    auto* hitbox = trap->AddComponent<HitboxComponent>();
    hitbox->SetActive(false);
    hitbox->SetDamage(config.damage);
    hitbox->SetKnockbackForce(config.knockback);
    hitbox->SetRetriggerCooldown(config.retriggerCooldown);

    auto* trapComponent = trap->AddComponent<TrapComponent>();
    trapComponent->SetTelegraph(config.telegraph);
    trapComponent->SetArmed(config.armedDuration);
    trapComponent->SetCooldown(config.cooldownDuration);

    auto* arrowSound = trap->AddComponent<SoundComponent>();
    arrowSound->SetIs3d(true);
    arrowSound->SetMaxDistance(12.0f);
    arrowSound->SetVolume(5.0f);

    trap->SetLocallyActive(false);
    return trap;
}
Entity* TrapSpawnerComponent::BuildFireTrap(Scene* scene, const std::string& name)
{
    Entity* trap = scene->AddEntity(name);

    auto* transform = trap->AddComponent<TransformComponent>();
    transform->SetLocalScale({0.4f, 0.4f, 0.4f});

    auto* model = trap->AddComponent<ModelComponent>();
    model->SetModel(this->fireTrapModel);

    auto* boxCollider = trap->AddComponent<BoxCollisionComponent>();
    boxCollider->SetLayer(CollisionLayer::TRIGGER_HAZARD);
    boxCollider->SetTrigger(true);
    boxCollider->SetStatic(false);
    boxCollider->SetExtents({ 0.3f, 0.4f, 0.3f });
    boxCollider->SetCenterOffset({ 0.0f, 0.1f, 0.0f });

    auto* hitbox = trap->AddComponent<HitboxComponent>();
    hitbox->SetActive(false);
    hitbox->SetDamage(this->fireDamage);
    hitbox->SetKnockbackForce(0.0f);
    hitbox->SetRetriggerCooldown(this->fireDamageInterval);

    auto* tc = trap->AddComponent<TrapComponent>();
    tc->SetTelegraph(this->fireTelegraph);
    tc->SetArmed(this->fireArmedDuration);


    /*Entity* flame = scene->AddEntity(name + "_Flame");
    flame->SetParent(trap);
    flame->AddComponent<TransformComponent>()->SetLocalPosition({ 0.0f, 0.45f, 0.0f });

    auto* ps = flame->AddComponent<ParticleSystemComponent>();
    ps->SetAtlasTexture(this->flameTexture);
    ps->SetAtlasSize({ 12.0f, 6.0f });
    ps->SetCount(3);
    ps->SetLifeTime(1.0f);
    ps->SetSpawnRadius(0.6f);
    ps->SetStartScale(0.9f);
    ps->SetEndScale(0.9f);
    ps->SetIsAnimate(true);
    ps->SetIsAdditive(true);
    ps->SetAnimationSpeed(1.2f);
    ps->SetVelocity({ 0.0f, 0.5f, 0.0f });
    ps->SetStartTint({ 1.0f, 0.55f, 0.15f });
    ps->SetEndTint({ 0.4f, 0.05f, 0.0f });*/

    trap->SetLocallyActive(false);
    return trap;
}

bool TrapSpawnerComponent::SpawnOneFireTrap(const AppContext& context)
{
    if (this->fireTrapPool.empty()) return false;

    DirectX::XMFLOAT3 origin = { 0.0f, -2.48f, 0.0f };

    DirectX::XMFLOAT3 position;
    if (!SampleSpawnPosition(origin,
        this->spawnInnerRadius, this->spawnOuterRadius, position))
    {
        return false;
    }

    Entity* trap = this->fireTrapPool.back();
    this->fireTrapPool.pop_back();

    trap->GetComponent<TransformComponent>()->SetWorldPosition(position);
    trap->GetComponent<TrapComponent>()->Reset();
    trap->SetLocallyActive(true);

    this->activeFireTraps.push_back(trap);
    return true;
}

void TrapSpawnerComponent::CleanupExpiredTraps()
{
    for (auto it = this->activeFireTraps.begin(); it != this->activeFireTraps.end(); )
    {
        Entity* e = *it;
        if (!e) { it = this->activeFireTraps.erase(it); continue; }

        auto* tc = e->GetComponent<TrapComponent>();
        if (tc && tc->IsExpired())
        {
            e->SetLocallyActive(false);
            tc->Reset();
            this->fireTrapPool.push_back(e);
            it = this->activeFireTraps.erase(it);
        }
        else ++it;
    }
}

void TrapSpawnerComponent::OnStart(const AppContext& context)
{
    this->spikeTrapModel = context.assetManager->Get<Model>("assets/models/traps/spiketrap.glb"); // TODO: find a spike trap
    this->fireTrapModel = context.assetManager->Get<Model>("assets/models/traps/spike_trap_01.glb"); // TODO: find a fire trap
    this->flameTexture = context.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png");
    this->smokeTexture = context.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png");

    this->aiManager = context.aiManager;
    this->InitializeTrapPools(context, this->GetOwner()->GetScene());
}

void TrapSpawnerComponent::Update(const AppContext & context, float deltaTime)
{
    if (!this->aiManager) {
        this->aiManager = context.aiManager;
    }
}

void TrapSpawnerComponent::OnDestroy(const AppContext& context)
{
    this->spikeTrapModel.reset();
    this->fireTrapModel.reset();
    this->flameTexture.reset();
    this->smokeTexture.reset();
}

void TrapSpawnerComponent::SpawnSpikeTraps(const SpikeTrapConfig& config)
{
    if (config.amount == 0) return;

    if (this->aiManager)
    {
        this->aiManager->GetGrid().ClearTraps();
    }
    Scene* scene = this->GetOwner()->GetScene();
    Entity* spikeRoot = scene->AddEntity("SpikeTraps");
    DirectX::XMFLOAT3 origin = { 0.0f, -0.5f, 0.0f };


    for (uint32_t i = 0; i < config.amount; ++i)
    {
        DirectX::XMFLOAT3 position;
        if (!this->SampleSpawnPosition(origin, this->spawnInnerRadius, this->spawnOuterRadius, position))
        {
            continue;
        }

        Entity* trap = this->BuildSpikeTrap(scene, "SpikeTrap_" + std::to_string(i), config);

        trap->SetParent(spikeRoot);
        trap->GetComponent<TransformComponent>()->SetWorldPosition(position);
        trap->GetComponent<TrapComponent>()->Reset();
        trap->SetLocallyActive(true);
        this->activeSpikeTraps.push_back(trap);

        // Register the trap into the AI grid!
        if (this->aiManager)
        {
            this->aiManager->GetGrid().InsertTrap(position);
        }
    }
}

void TrapSpawnerComponent::BeginWave()
{
    this->fireTimer = (this->maxFireTraps > 0)
        ? this->fireSpawnDuration / this->maxFireTraps
        : 0.0f;
    this->fireElapsed = 0.0f;
    this->firesSpawned = 0;
}
