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
	ExportProperty(int, maxEnemiesAlive, 4);
	ExportProperty(int, currentEnemies, 4);
	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);
	ExportProperty(float, enemyMoveSpeed, 3.0f);
	ExportProperty(float, enemyHealth, 100.0f);

	std::shared_ptr<Model> gibModel[3];
	std::shared_ptr<Model> enemyModel;
	std::shared_ptr<Texture2D> atlasTexture;


private:
	float gibTimer = 0.0f;
	float timer = 0.0f;
	std::vector<Entity*> spawnedEnemies;
	std::vector<Entity*> gibsSpawned;

	void SpawnEnemy(const AppContext& context, int numberOfEnemies);
	void SpawnGibs(const AppContext& context, DirectX::XMFLOAT3 pos, Scene* scene);
	void CleanGibs(Scene* scene);

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