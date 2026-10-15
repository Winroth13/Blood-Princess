#pragma once
#include "scene/component.hpp"
#include "assets/types/audio_clip.hpp"
#include "components/core/pool_component.hpp"

#include <DirectXMath.h>
#include <vector>

class Entity;
class Enemy;
class GibsComponent;
class AIManager;

class EnemySpawnerComponent : public Component
{
	RegisterComponent(EnemySpawnerComponent);

	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);

	ExportProperty(float, deathBloodRadiusMin, 1.5f);
	ExportProperty(float, deathBloodRadiusMax, 2.5f);

	const size_t GOBLIN = 0;
	const size_t OGRE = 1;
	const size_t SKELETON = 2;

	std::array<std::shared_ptr<Enemy>, 2> enemies = { nullptr };
	std::array<PoolComponent *, 2> enemyPools = { nullptr };

	std::array<int, 2> enemyPoolSize = 
	{
		256,
		8
	};

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
	bool SpawnEnemy(size_t type);

	void SetGibsComponent(GibsComponent *gibs) { this->gibs = gibs; }
};