#include "enemy_database_component.hpp"
#include "assets/asset_manager.hpp"

#include <format>

void EnemyDatabaseComponent::OnStart(const AppContext &context)
{
	for (size_t i = 0; i < +EnemyType::COUNT; ++i)
	{
		EnemyType type = static_cast<EnemyType>(i);

		std::shared_ptr<Enemy> enemy = 
			context.assetManager->Get<Enemy>(std::format("assets/enemies/{}.yaml", Enemy::GetStringFromEnemyType(type)));

		this->enemies[i] = enemy;
	}
}
