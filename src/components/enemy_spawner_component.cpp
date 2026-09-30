#include "enemy_spawner_component.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "components/box_collision_component.hpp"
#include "components/health_component.hpp"
#include "components/hurt_box_component.hpp"
#include "components/hit_box_component.hpp"
#include "components/enemy_controller_component.hpp"
#include "scene/scene.hpp"
#include "assets/asset_manager.hpp"
#include "components/model_component.hpp"
#include "components/animator_component.hpp"

void EnemySpawnerComponent::SpawnEnemy(const AppContext& context)
{
	TransformComponent* player = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players")->GetComponent<TransformComponent>();
	Scene* mainScene = this->GetOwner()->GetScene();
	DirectX::XMFLOAT3 orgin = 
		player ? 
		DirectX::XMFLOAT3{ player->GetWorldPosition().x , player->GetWorldPosition().y - 1.5f  ,player->GetWorldPosition().z } :
		DirectX::XMFLOAT3{ 0.0f,-2.0f,0.0f };

	for (size_t i = 0; i < this->maxEnemiesAlive; i++)
	{
		Entity* enemy = mainScene->AddEntity("Enemy_" + std::to_string(rand()));
		float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI;
		float dist = ((float)rand() / RAND_MAX) * this->spawnRadius;

		DirectX::XMFLOAT3 position = {
			orgin.x + cosf(angle) * dist,
			orgin.y,
			orgin.z + sinf(angle) * dist
		};
		TransformComponent* enemyTransform = enemy->AddComponent< TransformComponent>();;
		enemyTransform->SetWorldPosition(position);
		enemyTransform->SetWorldScale(0.5f);


		ModelComponent* modelComponent = enemy->AddComponent<ModelComponent>();
		std::shared_ptr<Model> model = context.assetManager->Get<Model>("assets/models/animations/princess_twerk.glb");
		modelComponent->SetModel(model);


		AnimatorComponent* animator = enemy->AddComponent<AnimatorComponent>();
		animator->AddAnimation("twerk", model->GetAnimations());
		animator->SetSkeletonVisible(true);
		animator->Play("twerk", true);


		Health* enemyHealth = enemy->AddComponent<Health>();
		enemyHealth->SetMaxHealth(this->enemyHealth);
		enemyHealth->SetCurrentHealth(this->enemyHealth);

		BoxCollisionComponent* enemyBC = enemy->AddComponent<BoxCollisionComponent>();
		enemyBC->SetExtents({ 1,1,1 });
		enemyBC->SetLayer(CollisionLayer::ENEMY);
		
		HurtboxComponent* enemyHurtbox = enemy->AddComponent<HurtboxComponent>();
		
		EnemyControllerComponent* enemyController = enemy->AddComponent<EnemyControllerComponent>();
		//enemyController->SetAvoidanceRadius()
		enemyController->SetMoveSpeed(this->enemyMoveSpeed);
		

		spawnedEnemies.push_back(enemy);
	}
}

void EnemySpawnerComponent::CleanupDeadEnemies()
{
	this->spawnedEnemies.erase(
		std::remove_if(this->spawnedEnemies.begin(), this->spawnedEnemies.end(),
			[](Entity* e) {
				if (!e) return true;
				auto* hp = e->GetComponent<Health>();
				if (hp && hp->IsDead()) {
					e->GetScene()->DestroyEntity(e);
					return true;
				}
				return false;
			}),
		this->spawnedEnemies.end()
	);
}

void EnemySpawnerComponent::OnStart(const AppContext& context)
{
	this->timer = this->spawnerInterval;
}

void EnemySpawnerComponent::Update(const AppContext& context, float deltaTime)
{
	this->CleanupDeadEnemies();

	if (static_cast<int>(this->spawnedEnemies.size()) >= this->maxEnemiesAlive)
		return;

	this->timer -= deltaTime;

	if (this->timer <= 0.0f) {
		this->timer = this->spawnerInterval;
		SpawnEnemy(context);
	}
}
