#pragma once
#include "scene/component.hpp"
#include <DirectXMath.h>
#include <memory>
#include <vector>

class Entity;
class Model;
class Texture2D;



class TrapSpawnerComponent : public Component
{
    RegisterComponent(TrapSpawnerComponent);

    // Spike traps
    ExportProperty(int, maxSpikeTraps, 10);
    ExportProperty(float, spikeSpawnDuration, 12.0f);
    ExportProperty(float, spikeTelegraph, 1.2f);
    ExportProperty(float, spikeArmedDuration, 4.0f);
    ExportProperty(float, spikeDamage, 25.0f);
    ExportProperty(float, spikeKnockback, 3.0f);
    ExportProperty(int, maxSpikePool, 12);

    // Fire traps
    ExportProperty(int, maxFireTraps, 5);
    ExportProperty(float, fireSpawnDuration, 20.0f);
    ExportProperty(float, fireTelegraph, 2.0f);
    ExportProperty(float, fireArmedDuration, 8.0f);
    ExportProperty(float, fireDamage, 10.0f);
    ExportProperty(float, fireDamageInterval, 0.5f);
    ExportProperty(int, maxFirePool, 10);

    // Shared
    ExportProperty(float, spawnInnerRadius, 3.0f);
    ExportProperty(float, spawnOuterRadius, 10.0f);

    std::shared_ptr<Model>     spikeTrapModel;
    std::shared_ptr<Model>     fireTrapModel;
    std::shared_ptr<Texture2D> flameTexture;
    std::shared_ptr<Texture2D> smokeTexture;

    // Timers
    float spikeTimer = 0.0f;
    float spikeElapsed = 0.0f;
    int   spikesSpawned = 0;

    float fireTimer = 0.0f;
    float fireElapsed = 0.0f;
    int   firesSpawned = 0;

    // Pools
    std::vector<Entity*> spikeTrapPool;
    std::vector<Entity*> activeSpikeTraps;
    std::vector<Entity*> fireTrapPool;
    std::vector<Entity*> activeFireTraps;

    void InitializeTrapPools(const AppContext& context, Scene* scene);
    Entity* BuildSpikeTrap(Scene* scene, const std::string& name);
    Entity* BuildFireTrap(Scene* scene, const std::string& name);

    bool SpawnOneSpikeTrap(const AppContext& context);
    bool SpawnOneFireTrap(const AppContext& context);

    void CleanupExpiredTraps();

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;

public:
    TrapSpawnerComponent(Entity* owner, bool isActive)
        : Component(owner, isActive) {
        UsesUpdate();
    }
    ~TrapSpawnerComponent() = default;

    void SetSpikeMaxAlive(int n) { this->maxSpikeTraps = n; }
    void SetSpikeSpawnDuration(float s) { this->spikeSpawnDuration = s; }
    void SetFireMaxAlive(int n) { this->maxFireTraps = n; }
    void SetFireSpawnDuration(float s) { this->fireSpawnDuration = s; }

    void BeginWave();
};