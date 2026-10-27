#pragma once
#include "scene/component.hpp"
#include "assets/types/audio_clip.hpp"
#include "components/core/pool_component.hpp"
#include "assets/types/enemy.hpp"
#include "events/signal.hpp"

#include <DirectXMath.h>
#include <vector>
class Entity;
class GibsComponent;
class AIManager;

class EnemySpawnerComponent : public Component
{
	RegisterComponent(EnemySpawnerComponent);

	Signal<EnemyType, Entity *> onEnemySpawned;
	Signal<EnemyType, uint32_t> onEnemyKilled;

	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);

	ExportProperty(bool, autoStartWave, true);

	std::array<PoolComponent *, 4> enemyPools = { nullptr };

    std::shared_ptr<Model> eyeModel = nullptr;
	std::shared_ptr<Model> knifeModel = nullptr;
	std::shared_ptr<Model> clubModel = nullptr;
	std::shared_ptr<Texture2D> sparksTexture;

private:
	GibsComponent *gibs = nullptr;
	AIManager *aiManager = nullptr;

	void InitializePools(const AppContext &context, Scene *scene);

	Entity *CreateEnemyEntity(std::shared_ptr<Enemy> enemy, Scene *scene);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void OnDestroy(const AppContext& context) override;

public:
	EnemySpawnerComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~EnemySpawnerComponent() = default;

	/*
	*	Tries to spawn an enemy at a suitable location somewhere around the player.
	*	Returns whether it successful in spawning the enemy.
	*/
	bool SpawnEnemy(size_t type, uint32_t wave);

	void SetGibsComponent(GibsComponent *gibs) { this->gibs = gibs; }

	Signal<EnemyType, uint32_t> &GetOnEnemyKilled() { return this->onEnemyKilled; }
	Signal<EnemyType, Entity *> &GetOnEnemySpawned() { return this->onEnemySpawned; }
};