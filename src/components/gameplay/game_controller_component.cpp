#include "game_controller_component.hpp"
#include "events/event_bus.hpp"
#include "scene/scene_manager.hpp"

void GameControllerComponent::SetPlayerHealthComponent(const AppContext &context, Health *health)
{
	SceneManager *sceneManager = context.sceneManager;
	health->GetOnDeath().Connect([sceneManager]() {
		sceneManager->RequestSceneChange("death_screen");
		});
}

void GameControllerComponent::SetEnemySpawnerComponent(const AppContext &context, EnemySpawnerComponent *enemySpawner)
{
	SceneManager *sceneManager = context.sceneManager;
	// TODO: Fix once EnemySpawnerComponent has been refactored
	//enemySpawner->GetOnEnemyDeath().Connect([sceneManager]() {
	//	sceneManager->RequestSceneChange("win_screen");
	//	});
}
