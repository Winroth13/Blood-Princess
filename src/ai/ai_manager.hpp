#pragma once

#include "ai/spatial_grid.hpp"

#include <DirectXMath.h>
#include <vector>

class EnemyControllerComponent;

class AIManager
{
private:
	std::vector<EnemyControllerComponent *> enemies;
	std::vector<EnemyControllerComponent*> pendingRemovals;
	SpatialGrid spatialGrid;

public:
	AIManager();
	~AIManager() = default;

	AIManager(const AIManager &other) = delete;
	AIManager &operator=(const AIManager &other) = delete;

	bool Initialize();
	void Shutdown();

	void RegisterEnemy(EnemyControllerComponent *enemy);
	void QueueUnregister(EnemyControllerComponent* enemy);
	void UnregisterEnemy(EnemyControllerComponent *enemy);

	void Update(float deltaTime);

	const SpatialGrid &GetGrid() const { return this->spatialGrid; }
	SpatialGrid &GetGrid() { return this->spatialGrid; }
};
