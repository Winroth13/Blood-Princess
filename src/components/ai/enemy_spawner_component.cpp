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
#include "components/gameplay/paintable_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
#include "components/gameplay/splatter_component.hpp"


uint32_t s_gibCounter = 0;

void EnemySpawnerComponent::InitializeGibPool(const AppContext& context, Scene* scene)
{
    gibPool.reserve(maxGibsPool);
    activeGibs.reserve(maxGibsPool);

    bool ribCageSpawned = false;

    Entity *gibs = scene->AddEntity("Gibs");

    for (int i = 0; i < maxGibsPool; ++i)
    {
        Entity* gib = scene->AddEntity("Pooled_Gib_" + std::to_string(s_gibCounter++));
        gib->SetParent(gibs);

        auto* gibTransform = gib->AddComponent<TransformComponent>();
        auto* modelComponent = gib->AddComponent<ModelComponent>();
        auto* gibParticle = gib->AddComponent<ParticleSystemComponent>();
        auto* projectileGib = gib->AddComponent<ProjectileComponent>();
        auto* sphereCollisionGib = gib->AddComponent<SphereCollisionComponent>();
        auto* splatterGib = gib->AddComponent<SplatterComponent>();


        std::shared_ptr<Model> model;
        const int ribCageNum = RNG::Int(1, 10);
        if (ribCageNum < 5) {
            model = gibModel[0];
            gibTransform->SetWorldScale(RNG::Float(0.455f, 0.78f));
        }
        else if (ribCageNum < 9) {
            model = gibModel[1];
            gibTransform->SetWorldScale(RNG::Float(0.195f, 0.325f));
        }
        else if (!ribCageSpawned) {
            model = gibModel[2];
            gibTransform->SetWorldScale(RNG::Float(0.91f, 1.17f));
            ribCageSpawned = true;
        }
        else {
            model = gibModel[1];
            gibTransform->SetWorldScale(RNG::Float(0.195f, 0.325f));
        }
        modelComponent->SetModel(model);


        if (model == gibModel[2]) {
            gibParticle->SetLifeTime(0.0f);
        }
        else {
            gibParticle->SetLifeTime(RNG::Float(0.3f, 0.8f));
        }

        gibParticle->SetSpawnRadius(RNG::Float(0.05f, 0.35f));
        gibParticle->SetCount(RNG::Int(8, 16));
        gibParticle->SetIsAnimate(true);
        gibParticle->SetIsGlobal(true);
        gibParticle->SetIsAdditive(false);
        gibParticle->SetStartTint({ 0.35f, 0.03f, 0.03f });
        gibParticle->SetEndTint({ 0.15f, 0.02f, 0.02f });
        gibParticle->SetAtlasTexture(this->atlasTexture);
        gibParticle->SetAtlasSize({ 8.0f, 8.0f });

        const float pAngle = RNG::Float(0.0f, DirectX::XM_2PI);
        const float pSpeed = RNG::Float(1.0f, 2.0f);
        gibParticle->SetVelocity({
            cosf(pAngle) * pSpeed,
            RNG::Float(0.1f, 0.3f),
            sinf(pAngle) * pSpeed
            });
        gibParticle->SetAcceleration({ 0.0f, -9.82f, 0.0f });

        const float pScale = RNG::Float(0.3f, 0.65f);
        gibParticle->SetStartScale(pScale);
        gibParticle->SetEndScale(pScale);


        projectileGib->SetGravity(9.82f);
        projectileGib->SetRestitution(RNG::Float(0.3f, 0.45f));


        sphereCollisionGib->SetRadius(RNG::Float(0.19f, 0.39f));
        sphereCollisionGib->SetLayer(CollisionLayer::GIBS);
        sphereCollisionGib->SetActive(true);
        sphereCollisionGib->SetStatic(false);
        sphereCollisionGib->SetTrigger(true);

        splatterGib->SetPaintSpacing(RNG::Float(0.4f, 1.0f));


        gib->SetLocallyActive(false);
        gibPool.push_back(gib);
    }
}

void EnemySpawnerComponent::InitializeEnemyPool(const AppContext& context, Scene* scene)
{
    enemyPool.reserve(maxEnemyPool);
    activeEnemy.reserve(maxEnemyPool);

    Entity *enemies = scene->AddEntity("Entities");

    for (int i = 0; i < maxEnemyPool; ++i)
    {
        Entity* enemy = scene->AddEntity("Pooled_Enemy_" + std::to_string(RNG::Int(0, 2000000)));
        enemy->SetParent(enemies);

        auto* enemyTransform = enemy->AddComponent<TransformComponent>();
        auto* modelComponent = enemy->AddComponent<ModelComponent>();
        auto* animator = enemy->AddComponent<AnimatorComponent>();
        auto* enemyHealth = enemy->AddComponent<Health>();
        auto* enemyBC = enemy->AddComponent<BoxCollisionComponent>();
        auto* enemyHurtbox = enemy->AddComponent<HurtboxComponent>();
        auto* enemyController = enemy->AddComponent<EnemyControllerComponent>();

        enemyTransform->SetWorldScale(0.9f);
        modelComponent->SetModel(enemyModel);

        animator->AddAnimation("run", enemyModel->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("run", true);

        enemyBC->SetExtents({ 1.2f, 1.7f, 0.45f });
        enemyBC->SetCenterOffset({ 0.0f, 1.8f, 0.0f });
        enemyBC->SetLayer(CollisionLayer::ENEMY);


        enemyController->SetMoveSpeed(this->enemyMoveSpeed);

        enemy->SetLocallyActive(false);
        enemyPool.push_back(enemy);
    }
}



void EnemySpawnerComponent::SpawnGibsFromPool(const AppContext& context, DirectX::XMFLOAT3 pos)
{
    const int numberOfGibs = RNG::Int(3, 5);
    pos.y += 2.0f;

    for (int i = 0; i < numberOfGibs; ++i)
    {
        
        if (this->gibPool.empty()) {
            if (this->activeGibs.empty()) break;
            Entity* oldestGib = this->activeGibs.front();
            this->activeGibs.erase(this->activeGibs.begin());
            this->gibPool.push_back(oldestGib);
        }

        Entity* gib = this->gibPool.back();
        this->gibPool.pop_back();


        TransformComponent* gibTransform = gib->GetComponent<TransformComponent>();
        gibTransform->SetWorldPosition(pos);
        gib->SetLocallyActive(true);


        ProjectileComponent* projectileGib = gib->GetComponent<ProjectileComponent>();
        projectileGib->SetAliveTime(RNG::Float(4.0f, 8.0f));


        const float gAngle = RNG::Float(0.0f, DirectX::XM_2PI);
        const float horizontalSpeed = RNG::Float(1.5f, 4.5f);
        const float upwardSpeed = RNG::Float(4.0f, 6.0f);

        DirectX::XMFLOAT3 velocity = {
            cosf(gAngle) * horizontalSpeed,
            upwardSpeed,
            sinf(gAngle) * horizontalSpeed
        };

        DirectX::XMFLOAT3 angularVelocity = {
            RNG::Float(-360.0f, 360.0f),
            RNG::Float(-360.0f, 360.0f),
            RNG::Float(-360.0f, 360.0f)
        };


        projectileGib->Launch(velocity, angularVelocity);
        if (i == 0) {
            gib->GetComponent<SplatterComponent>()->SetFirstPaint(pos);
        }

        this->activeGibs.push_back(gib);
    }
}

void EnemySpawnerComponent::SpawnEnemyFromPool(const AppContext& context, int numberOfEnemies)
{
    TransformComponent* player = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players")->GetComponent<TransformComponent>();
    DirectX::XMFLOAT3 origin =
        player ?
        DirectX::XMFLOAT3{ player->GetWorldPosition().x, player->GetWorldPosition().y - 1.5f, player->GetWorldPosition().z } :
        DirectX::XMFLOAT3{ 0.0f, -2.0f, 0.0f };

    for (int i = 0; i < numberOfEnemies; i++)
    {
        if (this->enemyPool.empty()) break;

        Entity* enemy = this->enemyPool.back();
        this->enemyPool.pop_back();

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


        TransformComponent* enemyTransform = enemy->GetComponent<TransformComponent>();
        enemyTransform->SetWorldPosition(position);


        Health* enemyHealth = enemy->GetComponent<Health>();
        enemyHealth->SetMaxHealth(this->enemyHealth);
        enemyHealth->SetCurrentHealth(this->enemyHealth);

        AnimatorComponent* animator = enemy->GetComponent<AnimatorComponent>();
        animator->SetTime(static_cast<float>(rand() % 100));

        enemy->SetLocallyActive(true);
        activeEnemy.push_back(enemy);
    }
}



void EnemySpawnerComponent::CleanGibs()
{
    for (auto it = activeGibs.begin(); it != activeGibs.end(); )
    {
        Entity* gib = *it;
        if (!gib) {
            it = activeGibs.erase(it);
            continue;
        }

        auto* projectile = gib->GetComponent<ProjectileComponent>();
        if (projectile && projectile->GetAliveTime() <= 0.0f)
        {
            gib->SetLocallyActive(false);
            this->gibPool.push_back(gib);

            it = activeGibs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EnemySpawnerComponent::CleanupDeadEnemies(const AppContext& context)
{
    for (auto it = activeEnemy.begin(); it != activeEnemy.end(); )
    {
        Entity* e = *it;
        if (!e) {
            it = activeEnemy.erase(it);
            continue;
        }

        auto* hp = e->GetComponent<Health>();
        if (hp && hp->IsDead())
        {
            if (auto* transform = e->GetComponent<TransformComponent>()) {
                SpawnGibsFromPool(context, transform->GetWorldPosition());
            }


            e->SetLocallyActive(false);
            this->enemyPool.push_back(e);

            it = activeEnemy.erase(it);
        }
        else
        {
            ++it;
        }
    }

    this->currentEnemies = static_cast<int>(this->activeEnemy.size());
}


void EnemySpawnerComponent::OnStart(const AppContext& context)
{
	this->timer = this->spawnerInterval;
	this->gibModel[0] = context.assetManager->Get<Model>("assets/models/gib1.glb");
	this->gibModel[1] = context.assetManager->Get<Model>("assets/models/gib2.glb");
	this->gibModel[2] = context.assetManager->Get<Model>("assets/models/gib3.glb");

	this->enemyModel = context.assetManager->Get<Model>("assets/models/goblin.glb");

	this->atlasTexture = context.assetManager->Get<Texture2D>("assets/images/detailed_smoke_spritesheet.png");


	this->InitializeGibPool(context, this->GetOwner()->GetScene());
    this->InitializeEnemyPool(context, this->GetOwner()->GetScene());
}

void EnemySpawnerComponent::Update(const AppContext& context, float deltaTime)
{
    this->CleanupDeadEnemies(context);
    this->CleanGibs();


    if (this->currentEnemies >= this->maxEnemiesAlive)
        return;

    this->timer -= deltaTime;

    if (this->timer <= 0.0f) {
        this->timer = this->spawnerInterval;


        int enemiesToSpawn = this->maxEnemiesAlive - this->currentEnemies;
        SpawnEnemyFromPool(context, enemiesToSpawn);
    }
}

void EnemySpawnerComponent::OnDestroy(const AppContext& context)
{
	for (size_t i = 0; i < 3; i++)
	{
		gibModel[i].reset();
	}

	this->enemyModel.reset();
}
