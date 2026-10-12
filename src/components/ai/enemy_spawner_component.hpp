#pragma once
#include "scene/component.hpp"
#include <DirectXMath.h>
#include <vector>
#include "assets/types/audio_clip.hpp"
class Entity;

class EnemySpawnerComponent : public Component
{
	RegisterComponent(EnemySpawnerComponent);

	// ogre start
	ExportProperty(int, maxOgreEnemies, 4);
	ExportProperty(float, ogreSpawnDuration, 10.0f);
	ExportProperty(float, ogreMoveSpeed, 2.0f);
	ExportProperty(float, ogreHealth, 500.0f);
	// ogre end


	ExportProperty(int, maxEnemiesAlive, 30);
	ExportProperty(float, enemySpawnDuration, 10.0f);
	ExportProperty(int, currentEnemies, 1);

	ExportProperty(float, spawnOuterRadius, 20.0f);
	ExportProperty(float, spawnInnerRadius, 14.0f);
	ExportProperty(float, enemyMoveSpeed, 3.0f);
	ExportProperty(float, enemyHealth, 100.0f);

	ExportProperty(int, maxGibsPool, 100);
	ExportProperty(int, maxEnemyPool, 250);

	ExportProperty(float, deathBloodRadiusMin, 1.5f);
	ExportProperty(float, deathBloodRadiusMax, 2.5f);

	std::shared_ptr<Model> gibModel[4];
	std::shared_ptr<Texture2D> atlasTexture;
	
	std::shared_ptr<Model> enemyModel;

	// ogre
	std::shared_ptr<Model> ogreModel;

private:

	float goblinTimer = 0.0f;   // countdown to next goblin
	float goblinElapsed = 0.0f;   // seconds since goblin wave started
	int   goblinsSpawned = 0;

	float ogreTimer = 0.0f;
	float ogreElapsed = 0.0f;
	int   ogresSpawned = 0;

	float gibTimer = 0.0f;
	float timer = 0.0f;

	std::vector<Entity*> enemyPool;
	std::vector<Entity*> activeEnemy;

	std::vector<Entity*> activeOgre;

	std::vector<Entity*> gibPool;
	std::vector<Entity*> activeGibs;


	void InitializeGibPool(const AppContext& context, Scene* scene);
	void InitializeEnemyPool(const AppContext& context, Scene* scene);

	void SpawnGibsFromPool(const AppContext& context, DirectX::XMFLOAT3 pos, float count = 3);
	bool SpawnOneEnemyFromPool(const AppContext& context);
	bool SpawnOneOgre(const AppContext& context);


	void SpawnBloodExplosion(Scene *scene, XMFLOAT3 position);

	void CleanGibs();
	void CleanupDeadEnemies(const AppContext& context);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void OnDestroy(const AppContext& context) override;


public:

	EnemySpawnerComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~EnemySpawnerComponent() = default;


	void SetGoblinMaxAlive(int amount) { this->maxEnemiesAlive = amount; }
	void SetGoblinSpawnDuration(float amount) { this->enemySpawnDuration = amount; }

	void SetOgreMaxAlive(int amount) { this->maxOgreEnemies = amount; }
	void SetOgreSpawnDuration(float amount) { this->ogreSpawnDuration = amount; }

	void BeginWave();
};