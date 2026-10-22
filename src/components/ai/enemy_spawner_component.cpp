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
#include "components/core/lifetime_component.hpp"
#include "components/audio/sound_component.hpp"
#include "components/gameplay/gibs_component.hpp"
#include "components/gameplay/enemy_database_component.hpp"
#include "assets/types/sound.hpp"
#include "assets/types/enemy.hpp"
#include "core/rng.hpp"
#include "ai/ai_manager.hpp"

#include <format>

uint32_t s_gibCounter = 0;

void EnemySpawnerComponent::InitializePools(const AppContext &context, Scene *scene)
{
    for (size_t i = 0; i < +EnemyType::COUNT; ++i)
    {
        auto *database = this->GetOwner()->GetComponent<EnemyDatabaseComponent>();

        std::shared_ptr<Enemy> enemy = database->GetEnemy(static_cast<EnemyType>(i));

        PoolComponent *pool = this->GetOwner()->AddComponent<PoolComponent>();

        pool->SetCreateEntityCallback([this, enemy, scene, pool, i]() {
            EnemyType type = static_cast<EnemyType>(i);

            Entity *enemyEntity = this->CreateEnemyEntity(enemy, scene);

            auto health = enemyEntity->GetComponent<Health>()->GetOnDeath().Connect([this, pool, enemyEntity, type]() 
            {
                pool->Put(enemyEntity);

                EnemyControllerComponent *controller = enemyEntity->GetComponent<EnemyControllerComponent>();

                this->GetOnEnemyKilled().Emit(type, controller->GetWave());
            });

            return enemyEntity;
        });

        int poolSize = i == 0 ? 256 : 32;

        // TODO: CHANGE TO WIZARD LATER // done
        if (i == +EnemyType::WIZARD) poolSize = 1;
        pool->SetSize(poolSize);

        this->enemyPools[i] = pool;
    }
}

Entity *EnemySpawnerComponent::CreateEnemyEntity(std::shared_ptr<Enemy> enemy, Scene *scene)
{
    Entity *entity = scene->AddEntity(Enemy::GetStringFromEnemyType(enemy->type));

    auto *enemyTransform = entity->AddComponent<TransformComponent>();
    enemyTransform->SetWorldScale(enemy->scale);
    
    auto *modelComponent = entity->AddComponent<ModelComponent>();
    modelComponent->SetModel(enemy->model);

    auto *animator = entity->AddComponent<AnimatorComponent>();
    animator->AddAnimations(enemy->model->GetAnimations());
    animator->SetSkeletonVisible(true);
    animator->Play(enemy->walkAnimation, true);

    auto *enemyHealth = entity->AddComponent<Health>();
    enemyHealth->SetMaxHealth(enemy->health);

    auto *enemyBC = entity->AddComponent<BoxCollisionComponent>();
    enemyBC->SetExtents(enemy->extends);
    enemyBC->SetCenterOffset(enemy->extendsOffset);
    enemyBC->SetLayer(CollisionLayer::ENEMY);

    auto *enemyHurtbox = entity->AddComponent<HurtboxComponent>();

    auto *enemyController = entity->AddComponent<EnemyControllerComponent>();
    enemyController->SetGibs(this->gibs);
    enemyController->SetEnemy(enemy);

    auto *enemySplatter = entity->AddComponent<SplatterComponent>();

    auto *enemySoundComponent = entity->AddComponent<SoundComponent>();
    enemySoundComponent->SetSound(enemy->hitSound);

    Entity *hitboxEntity = scene->AddEntity("hitbox");
    hitboxEntity->SetParent(entity);

    auto *hitboxTransform = hitboxEntity->AddComponent<TransformComponent>();

    auto *hitboxCol = hitboxEntity->AddComponent<BoxCollisionComponent>();
    hitboxCol->SetLayer(CollisionLayer::ENEMY_ATTACK);
    hitboxCol->SetTrigger(true);
    hitboxCol->SetStatic(false);

    auto *hitbox = hitboxEntity->AddComponent<HitboxComponent>();
    hitbox->SetActive(false);

    return entity;
}

void EnemySpawnerComponent::OnStart(const AppContext& context)
{
    if (!this->gibs)
        LogError("Must have a gibs component");

    this->aiManager = context.aiManager;

    this->InitializePools(context, context.sceneManager->GetCurrentScene());
}

void EnemySpawnerComponent::Update(const AppContext& context, float deltaTime)
{

}

void EnemySpawnerComponent::OnDestroy(const AppContext& context)
{

}

bool EnemySpawnerComponent::SpawnEnemy(size_t type, uint32_t wave)
{
    Entity *entity = this->enemyPools[type]->Grab();

    // make sure there was any enemy to grab from the pool
    if (!entity)
        return false;

    Entity *playerEntity = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    TransformComponent *transform = playerEntity->GetComponent<TransformComponent>();

    XMFLOAT3 origin = transform->GetWorldPosition();
    origin.y = 0.0f;

    uint32_t tries = 0;

    XMFLOAT3 wishPosition = { 0.0f, 0.0f, 0.0f };
    do
    {
        float angle = RNG::Float() * DirectX::XM_2PI;
        float distance = RNG::Float() * (this->spawnOuterRadius - this->spawnInnerRadius) + this->spawnInnerRadius;

        wishPosition = {
            origin.x + std::cos(angle) * distance,
            origin.y,
            origin.z + std::sin(angle) * distance
        };

        // at some point we just have to give up!
        if (tries++ > 30)
        {
            this->enemyPools[type]->Put(entity);
            return false;
        }

    } while (!this->aiManager->GetGrid().IsAreaClear(wishPosition, 1.0f));

    entity->GetComponent<TransformComponent>()->SetWorldPosition(wishPosition);

    auto health = entity->GetComponent<Health>();
    health->SetCurrentHealth(health->GetMaxHealth());

    entity->GetComponent<EnemyControllerComponent>()->SetWave(wave);

    return true;
}
