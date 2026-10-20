#pragma once
#include "scene/component.hpp"
#include <DirectXMath.h>
#include <memory>
#include <vector>

class Entity;
class Model;
class Texture2D;
class AIManager;
struct SpikeTrapConfig;


class TrapSpawnerComponent : public Component
{
    RegisterComponent(TrapSpawnerComponent);

    // Fire traps
    ExportProperty(int, maxFireTraps, 0);
    ExportProperty(float, fireSpawnDuration, 20.0f);
    ExportProperty(float, fireTelegraph, 2.0f);
    ExportProperty(float, fireArmedDuration, 8.0f);
    ExportProperty(float, fireDamage, 10.0f);
    ExportProperty(float, fireDamageInterval, 0.5f);
    ExportProperty(int, maxFirePool, 10);

    // Shared
    ExportProperty(float, spawnInnerRadius, 3.0f);
    ExportProperty(float, spawnOuterRadius, 22.0f);
    ExportProperty(float, minTrapSpacing, 11.0f);

    std::shared_ptr<Model>     spikeTrapModel;
    std::shared_ptr<Model>     fireTrapModel;
    std::shared_ptr<Texture2D> flameTexture;
    std::shared_ptr<Texture2D> smokeTexture;

    // Timers

    float fireTimer = 0.0f;
    float fireElapsed = 0.0f;
    int   firesSpawned = 0;

    // Pools
    std::vector<Entity*> activeSpikeTraps;
    std::vector<Entity*> fireTrapPool;
    std::vector<Entity*> activeFireTraps;
    AIManager* aiManager = nullptr;

    void InitializeTrapPools(const AppContext& context, Scene* scene);
    Entity* BuildSpikeTrap(Scene* scene, const std::string& name, const SpikeTrapConfig& config);
    Entity* BuildFireTrap(Scene* scene, const std::string& name);

    bool SpawnOneFireTrap(const AppContext& context);

    bool SampleSpawnPosition(const DirectX::XMFLOAT3& origin, float innerR, float outerR,DirectX::XMFLOAT3& outPos);

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

    void SpawnSpikeTraps(const SpikeTrapConfig& config);
    void SetFireMaxAlive(int n) { this->maxFireTraps = n; }
    void SetFireSpawnDuration(float s) { this->fireSpawnDuration = s; }

    void BeginWave();
};