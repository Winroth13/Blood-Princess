#pragma once
#include "scene/component.hpp"
#include "assets/types/enemy.hpp"

#include <array>
#include <memory>

class EnemyDatabaseComponent : public Component
{
	RegisterComponent(EnemyDatabaseComponent);

	std::array<std::shared_ptr<Enemy>, +EnemyType::COUNT> enemies = { nullptr };

private:
	void OnStart(const AppContext &context) override;

public:
	EnemyDatabaseComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); UsesRender(); };
	~EnemyDatabaseComponent() = default;

	std::shared_ptr<Enemy> GetEnemy(EnemyType type) { return this->enemies[+type]; }
};