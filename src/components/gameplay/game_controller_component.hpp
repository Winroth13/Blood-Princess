#pragma once

#include "scene/component.hpp"
#include "components/gameplay/health_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"

class GameControllerComponent : public Component {
	RegisterComponent(GameControllerComponent);

public:
	GameControllerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesLateUpdate(); }

	void SetPlayerHealthComponent(const AppContext &context, Health *health);
	void SetEnemySpawnerComponent(const AppContext &context, EnemySpawnerComponent *enemySpawner);
};