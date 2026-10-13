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

// helper function
static bool SampleSpawnPosition(
    const AppContext& context,
    const DirectX::XMFLOAT3& origin,
    float innerR, float outerR,
    DirectX::XMFLOAT3& outPos)
{
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
        if (!context.aiManager || context.aiManager->GetGrid().IsAreaClear(outPos, 1.0f))
            return true;
    }
    return true;
}

void TrapSpawnerComponent::InitializeTrapPools(const AppContext& context, Scene* scene)
{
    this->spikeTrapPool.reserve(this->maxSpikePool);
    this->activeSpikeTraps.reserve(this->maxSpikePool);

    Entity* spikeRoot = scene->AddEntity("SpikeTraps");
    for (int i = 0; i < this->maxSpikePool; ++i)
    {
        Entity* t = BuildSpikeTrap(scene, "Pooled_SpikeTrap_" + std::to_string(i));
        t->SetParent(spikeRoot);
        this->spikeTrapPool.push_back(t);
    }


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

Entity* TrapSpawnerComponent::BuildSpikeTrap(Scene * scene, const std::string & name)
{
    Entity* trap = scene->AddEntity(name);

    auto* transform = trap->AddComponent<TransformComponent>();
    transform->SetLocalScale({3.75f,3.75f,3.75f});

    auto* model = trap->AddComponent<ModelComponent>();
    model->SetModel(this->spikeTrapModel);

    auto* boxCollider = trap->AddComponent<BoxCollisionComponent>();
    boxCollider->SetLayer(CollisionLayer::TRIGGER_HAZARD);
    boxCollider->SetTrigger(true);
    boxCollider->SetStatic(false);
    boxCollider->SetExtents({ 0.3f, 0.4f, 0.3f });
    boxCollider->SetCenterOffset({ 0.0f, 0.1f, 0.0f });

    auto* hitbox = trap->AddComponent<HitboxComponent>();
    hitbox->SetActive(false);
    hitbox->SetDamage(this->spikeDamage);
    hitbox->SetKnockbackForce(this->spikeKnockback);
    hitbox->SetRetriggerCooldown(0.0f);

    auto* trapComponent = trap->AddComponent<TrapComponent>();
    trapComponent->SetTelegraph(this->spikeTelegraph);
    trapComponent->SetArmed(this->spikeArmedDuration);

    trap->SetLocallyActive(false);
    return trap;
}

Entity* TrapSpawnerComponent::BuildFireTrap(Scene* scene, const std::string& name)
{
    Entity* trap = scene->AddEntity(name);

    auto* transform = trap->AddComponent<TransformComponent>();
    transform->SetLocalScale({ 3.75f,3.75f,3.75f });

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


    Entity* flame = scene->AddEntity(name + "_Flame");
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
    ps->SetEndTint({ 0.4f, 0.05f, 0.0f });

    trap->SetLocallyActive(false);
    return trap;
}

bool TrapSpawnerComponent::SpawnOneSpikeTrap(const AppContext& context)
{
    if (this->spikeTrapPool.empty()) return false;

    DirectX::XMFLOAT3 origin = { 0.0f, -2.48f, 0.0f };

    DirectX::XMFLOAT3 position;
    if (!SampleSpawnPosition(context, origin,
        this->spawnInnerRadius, this->spawnOuterRadius, position))
    {
        return false;
    }

    Entity* trap = this->spikeTrapPool.back();
    this->spikeTrapPool.pop_back();

    trap->GetComponent<TransformComponent>()->SetWorldPosition(position);
    trap->GetComponent<TrapComponent>()->Reset();
    trap->SetLocallyActive(true);

    this->activeSpikeTraps.push_back(trap);
    return true;
}

bool TrapSpawnerComponent::SpawnOneFireTrap(const AppContext& context)
{
    if (this->fireTrapPool.empty()) return false;

    DirectX::XMFLOAT3 origin = { 0.0f, -2.48f, 0.0f };

    DirectX::XMFLOAT3 position;
    if (!SampleSpawnPosition(context, origin,
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
    for (auto it = this->activeSpikeTraps.begin(); it != this->activeSpikeTraps.end(); )
    {
        Entity* e = *it;
        if (!e) { it = this->activeSpikeTraps.erase(it); continue; }

        auto* tc = e->GetComponent<TrapComponent>();
        if (tc && tc->IsExpired())
        {
            e->SetLocallyActive(false);
            tc->Reset();
            this->spikeTrapPool.push_back(e);
            it = this->activeSpikeTraps.erase(it);
        }
        else ++it;
    }

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
    this->spikeTrapModel = context.assetManager->Get<Model>("assets/models/traps/spike_trap_01.glb"); // TODO: find a spike trap
    this->fireTrapModel = context.assetManager->Get<Model>("assets/models/traps/spike_trap_01.glb"); // TODO: find a fire trap
    this->flameTexture = context.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png");
    this->smokeTexture = context.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png");

    this->InitializeTrapPools(context, this->GetOwner()->GetScene());
    for (int i = 0; i < this->maxSpikeTraps; ++i) this->SpawnOneSpikeTrap(context);
    for (int i = 0; i < this->maxFireTraps; ++i) this->SpawnOneFireTrap(context);
}

void TrapSpawnerComponent::Update(const AppContext & context, float deltaTime)
{
}

void TrapSpawnerComponent::OnDestroy(const AppContext& context)
{
    this->spikeTrapModel.reset();
    this->fireTrapModel.reset();
    this->flameTexture.reset();
    this->smokeTexture.reset();
}

void TrapSpawnerComponent::BeginWave()
{
    this->spikeTimer = (this->maxSpikeTraps > 0)
        ? this->spikeSpawnDuration / this->maxSpikeTraps
        : 0.0f;
    this->spikeElapsed = 0.0f;
    this->spikesSpawned = 0;

    this->fireTimer = (this->maxFireTraps > 0)
        ? this->fireSpawnDuration / this->maxFireTraps
        : 0.0f;
    this->fireElapsed = 0.0f;
    this->firesSpawned = 0;
}
