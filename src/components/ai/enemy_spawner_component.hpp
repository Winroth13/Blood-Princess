#pragma once
#include "scene/component.hpp"
#include <DirectXMath.h>
#include <vector>
#include "assets/types/audio_clip.hpp"
class Entity;

class EnemySpawnerComponent : public Component
{
	RegisterComponent(EnemySpawnerComponent);

	ExportProperty(float, spawnerInterval, 3.0f);
	ExportProperty(int, maxEnemiesAlive, 30);
	ExportProperty(int, currentEnemies, 1);
	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);
	ExportProperty(float, enemyMoveSpeed, 1.0f);
	ExportProperty(float, enemyHealth, 100.0f);

	ExportProperty(int, maxGibsPool, 100);
	ExportProperty(int, maxEnemyPool, 250);

	ExportProperty(float, deathBloodRadiusMin, 1.5f);
	ExportProperty(float, deathBloodRadiusMax, 2.5f);

	std::shared_ptr<Model> gibModel[4];
	std::shared_ptr<Model> enemyModel;
	std::shared_ptr<Texture2D> atlasTexture;


private:
	float gibTimer = 0.0f;
	float timer = 0.0f;

	std::vector<Entity*> enemyPool;
	std::vector<Entity*> activeEnemy;

	std::vector<Entity*> gibPool;
	std::vector<Entity*> activeGibs;


	void InitializeGibPool(const AppContext& context, Scene* scene);
	void InitializeEnemyPool(const AppContext& context, Scene* scene);

	void SpawnGibsFromPool(const AppContext& context, DirectX::XMFLOAT3 pos);
	void SpawnEnemyFromPool(const AppContext& context, int numberOfEnemies);

	void SpawnBloodExplosion(Scene *scene, XMFLOAT3 position);

	void CleanGibs();
	void CleanupDeadEnemies(const AppContext& context);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void LateUpdate(const AppContext& context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;


public:

	EnemySpawnerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(), UsesLateUpdate(), UsesRender(); };
	~EnemySpawnerComponent() = default;


};