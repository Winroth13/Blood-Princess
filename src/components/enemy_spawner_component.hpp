#pragma once
#include "scene/component.hpp"
#include <DirectXMath.h>
#include <vector>

class Entity;

class EnemySpawnerComponent : public Component
{
	RegisterComponent(EnemySpawnerComponent);

	ExportProperty(float, spawnerInterval, 3.0f);
	ExportProperty(int, maxEnemiesAlive, 4);
	ExportProperty(int, currentEnemies, 4);
	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);
	ExportProperty(float, enemyMoveSpeed, 3.0f);
	ExportProperty(float, enemyHealth, 100.0f);

private:
	float timer = 0.0f;
	std::vector<Entity*> spawnedEnemies;

	void SpawnEnemy(const AppContext& context, int numberOfEnemies);
	void CleanupDeadEnemies();

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void LateUpdate(const AppContext& context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override {};

public:

	EnemySpawnerComponent(Entity* owner, bool isActive) : Component(owner, isActive) {};
	~EnemySpawnerComponent() = default;
};