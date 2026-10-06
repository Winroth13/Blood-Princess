#include "enemy_spawner_component.hpp"
#include "scene/entity.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/gameplay/health_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/ai/enemy_controller_component.hpp"
#include "scene/scene.hpp"
#include "assets/asset_manager.hpp"
#include "components/core/model_component.hpp"
#include "components/skeletal/animator_component.hpp"
#include "core/logging.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "core/rng.hpp"
#include "scene/scene_manager.hpp"

void EnemySpawnerComponent::SpawnEnemy(const AppContext& context, int numberOfEnemies)
{
	TransformComponent* player = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players")->GetComponent<TransformComponent>();
	Scene* mainScene = this->GetOwner()->GetScene();
	DirectX::XMFLOAT3 origin = 
		player ? 
		DirectX::XMFLOAT3{ player->GetWorldPosition().x , player->GetWorldPosition().y - 1.5f  ,player->GetWorldPosition().z } :
		DirectX::XMFLOAT3{ 0.0f,-2.0f,0.0f };

	for (size_t i = 0; i < numberOfEnemies; i++)
	{
		Entity* enemy = mainScene->AddEntity("Enemy_" + std::to_string(i));
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		do
		{
			float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI;
			float dist = ((float)rand() / RAND_MAX) * (this->spawnOuterRadius - this->spawnInnerRadius) + this->spawnInnerRadius;

			position = {
				origin.x + cosf(angle) * dist,
				origin.y,
				origin.z + sinf(angle) * dist
			};

		} while (!context.aiManager->GetGrid().IsAreaClear(position, 1.0f));

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
		animator->SetTime(static_cast<float>(rand() % 100));

		Health* enemyHealth = enemy->AddComponent<Health>();
		enemyHealth->SetMaxHealth(this->enemyHealth);
		enemyHealth->SetCurrentHealth(this->enemyHealth);

		BoxCollisionComponent* enemyBC = enemy->AddComponent<BoxCollisionComponent>();
		enemyBC->SetExtents({ 1,1,1 });
		enemyBC->SetLayer(CollisionLayer::ENEMY);
		
		HurtboxComponent* enemyHurtbox = enemy->AddComponent<HurtboxComponent>();
		
		EnemyControllerComponent* enemyController = enemy->AddComponent<EnemyControllerComponent>();
		enemyController->SetMoveSpeed(this->enemyMoveSpeed);
		

		spawnedEnemies.push_back(enemy);
	}
}

void EnemySpawnerComponent::CleanupDeadEnemies(const AppContext& context)
{
	this->spawnedEnemies.erase(
		std::remove_if(this->spawnedEnemies.begin(), this->spawnedEnemies.end(),
			[&context, this](Entity* e) {
				if (!e) return true;

				auto* hp = e->GetComponent<Health>();
				if (hp && hp->IsDead()) {
					if (auto* transform = e->GetComponent<TransformComponent>()) {
						SpawnGib(context, transform->GetWorldPosition(), e->GetScene());
					}
					e->GetScene()->DestroyEntity(e);
					return true;
				}
				return false;
			}),
		this->spawnedEnemies.end()
	);

	this->currentEnemies = static_cast<int>(this->spawnedEnemies.size());
}


void EnemySpawnerComponent::OnStart(const AppContext& context)
{
	this->timer = this->spawnerInterval;
}

void EnemySpawnerComponent::Update(const AppContext& context, float deltaTime)
{
	this->CleanupDeadEnemies(context);
	this->CleanGibs(context.sceneManager->GetCurrentScene());

	if (static_cast<int>(this->spawnedEnemies.size()) >= this->maxEnemiesAlive)
		return;

	this->timer -= deltaTime;

	if (this->timer <= 0.0f) {
		this->timer = this->spawnerInterval;
		SpawnEnemy(context, this->maxEnemiesAlive - this->currentEnemies );
	}
}


void EnemySpawnerComponent::SpawnGib(const AppContext& context,DirectX::XMFLOAT3 pos, Scene* scene) {

	float numberOfGibs = RNG::Int(3, 10);
	for (size_t i = 0; i < numberOfGibs; i++)
	{
		Entity* gib = scene->AddEntity("Gib_" + std::to_string(i));

		TransformComponent* gibTransform = gib->AddComponent< TransformComponent>();;
		gibTransform->SetWorldPosition(pos);
		gibTransform->SetWorldScale(0.05f);


		ModelComponent* modelComponent = gib->AddComponent<ModelComponent>();
		std::shared_ptr<Model> model = context.assetManager->Get<Model>("assets/models/sphere.glb");
		modelComponent->SetModel(model);

		ProjectileComponent* projectileGib = gib->AddComponent<ProjectileComponent>();
		//projectileGib->SetAliveTime(RNG::Float(3, 10));
		projectileGib->SetAliveTime(5);

		DirectX::XMFLOAT3 velocity = { RNG::Float(0,10), RNG::Float(0,10) , RNG::Float(0,10) };
		DirectX::XMFLOAT3 angularVelocity = { RNG::Float(0,10), RNG::Float(0,10) , RNG::Float(0,10) };
		projectileGib->Launch(velocity, angularVelocity);

		DirectX::XMFLOAT3 particleAcceleration = { RNG::Float(0,10), RNG::Float(0,10) , RNG::Float(0,10) };
		ParticleSystemComponent* gibParticle = gib->AddComponent<ParticleSystemComponent>();
		float Scale = RNG::Float(0.8, 1.2);
		gibParticle->SetLifeTime(RNG::Float(0.1, 1));
		gibParticle->SetSpawnRadius(RNG::Float(0.001, 0.3));
		gibParticle->SetCount(RNG::Int(5,10));
		gibParticle->SetIsAnimate(false);
		gibParticle->SetVelocity(velocity);
		gibParticle->SetAcceleration(particleAcceleration);
		gibParticle->SetStartScale(Scale);
		gibParticle->SetEndScale(Scale);
		gibParticle->SetIsAdditive(false);
		gibParticle->SetStartTint({ 0.533f, 0.031f, 0.031f });
		gibParticle->SetEndTint({ 0.533f, 0.031f, 0.031f });
		gibParticle->SetAtlasTexture(context.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));

		gibsSpawned.push_back(gib);
	}
}
void EnemySpawnerComponent::CleanGibs(Scene* scene) {
	this->gibsSpawned.erase(
		std::remove_if(this->gibsSpawned.begin(), this->gibsSpawned.end(),
			[](Entity* e) {
				if (!e) return true;
				auto* gibs = e->GetComponent<ProjectileComponent>();
				if (gibs && gibs->GetAliveTime() <= 0.0f) {
					e->GetScene()->DestroyEntity(e);
					return true;
				}
				return false;
			}),
		this->gibsSpawned.end()
	);
};