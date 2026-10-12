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
#include "assets/types/sound.hpp"


uint32_t s_gibCounter = 0;

void EnemySpawnerComponent::InitializeGibPool(const AppContext& context, Scene* scene)
{
    gibPool.reserve(maxGibsPool);
    activeGibs.reserve(maxGibsPool);


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
        if (i < 40) {
            model = gibModel[0];
            gibTransform->SetWorldScale(RNG::Float(0.455f, 0.78f));
        }
        else if (i < 70) {
            model = gibModel[1];
            gibTransform->SetWorldScale(RNG::Float(0.195f, 0.325f));
        }
        else if (i < 85) {
            model = gibModel[2];
            gibTransform->SetWorldScale(RNG::Float(0.91f, 1.17f));
        }
        else {
            model = gibModel[3];
            gibTransform->SetWorldScale(RNG::Float(0.35f, 0.40f));
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

    auto goblinHitSound = context.assetManager->Get<Sound>("assets/sounds/goblin_hit_sound.sound");
    auto goblinRandomSound = context.assetManager->Get<Sound>("assets/sounds/goblin_random_sounds.sound");

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
        auto* enemySplatter = enemy->AddComponent<SplatterComponent>();
        auto* enemySoundComponent = enemy->AddComponent<SoundComponent>();


        enemyTransform->SetWorldScale(0.9f);
        modelComponent->SetModel(enemyModel);

        animator->AddAnimations(enemyModel->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("Jog", true);

        enemyBC->SetExtents({ 1.2f, 1.7f, 0.45f });
        enemyBC->SetCenterOffset({ 0.0f, 1.8f, 0.0f });
        enemyBC->SetLayer(CollisionLayer::ENEMY);


        enemyController->SetMoveSpeed(this->enemyMoveSpeed);
        enemyController->SetHitSound(goblinHitSound);
        enemyController->SetRandomSound(goblinRandomSound);
        enemySoundComponent->SetSound(goblinHitSound);

        enemy->SetLocallyActive(false);
        enemyPool.push_back(enemy);
    }
}

void EnemySpawnerComponent::SpawnGibsFromPool(const AppContext& context, DirectX::XMFLOAT3 pos, float count)
{
    int numberOfGibs = 0;
    if (count == 3) {
        numberOfGibs = RNG::Int(3, 5);
    }
    else
    {
        numberOfGibs = RNG::Int(6, 10);
    }
    pos.y += 2.0f;

    bool hasSpawnedRib = false;
    bool hasSpawnedHead = false;

    for (int i = 0; i < numberOfGibs; ++i)
    {
        int roll = RNG::Int(1, 100);
        int type = 0;
        if (roll <= 40) type = 0;
        else if (roll <= 70) type = 1;
        else if (roll <= 85 && !hasSpawnedRib) { type = 2; hasSpawnedRib = true; }
        else if (!hasSpawnedHead) { type = 3; hasSpawnedHead = true; }

        Entity* gib = nullptr;
        for (auto it = this->gibPool.begin(); it != this->gibPool.end(); ++it) {
            if ((*it)->GetComponent<ModelComponent>()->GetModel() == this->gibModel[type]) {
                gib = *it;
                this->gibPool.erase(it);
                break;
            }
        }

        if (!gib) {
            if (this->gibPool.empty()) {
                if (this->activeGibs.empty()) break;
                gib = this->activeGibs.front();
                this->activeGibs.erase(this->activeGibs.begin());
            }
            else {
                gib = this->gibPool.back();
                this->gibPool.pop_back();
            }
        }


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
        if (auto* splatter = gib->GetComponent<SplatterComponent>()) {
            splatter->Reset();
        }

        this->activeGibs.push_back(gib);
    }
}

bool EnemySpawnerComponent::SpawnOneEnemyFromPool(const AppContext& context)
{
    Entity* playerEntity = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    TransformComponent* player = nullptr;
    if (playerEntity) {
        player = playerEntity->GetComponent<TransformComponent>();
    }
    if (!player)       return false;

    DirectX::XMFLOAT3 origin =
        player ?
        DirectX::XMFLOAT3{ player->GetWorldPosition().x, player->GetWorldPosition().y - 1.5f, player->GetWorldPosition().z } :
        DirectX::XMFLOAT3{ 0.0f, -2.0f, 0.0f };

    if (this->enemyPool.empty()) return false;

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

    if (EnemyControllerComponent* enemyController = enemy->GetComponent<EnemyControllerComponent>())
    {
        enemyController->ResetKnockback();
    }

    AnimatorComponent* animator = enemy->GetComponent<AnimatorComponent>();
    animator->SetTime(static_cast<float>(rand() % 100));

    enemy->SetLocallyActive(true);
    activeEnemy.push_back(enemy);
    return true;
}

bool EnemySpawnerComponent::SpawnOneOgre(const AppContext& context)
{

    Entity* playerEntity = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    TransformComponent* player = nullptr;
    if (playerEntity) {
        player = playerEntity->GetComponent<TransformComponent>();
    }
    if (!player)       return false;

    Scene* mainScene = this->GetOwner()->GetScene();
    DirectX::XMFLOAT3 origin =
        player ?
        DirectX::XMFLOAT3{ player->GetWorldPosition().x , player->GetWorldPosition().y - 1.5f  ,player->GetWorldPosition().z } :
        DirectX::XMFLOAT3{ 0.0f,-2.0f,0.0f };

    Entity* ogre = mainScene->AddEntity("Enemy_Ogre_" + std::to_string(rand()));
    float angle = ((float)rand() / RAND_MAX) * DirectX::XM_2PI;
    float dist = ((float)rand() / RAND_MAX) * (this->spawnOuterRadius - this->spawnOuterRadius) + this->spawnOuterRadius;

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

    TransformComponent* ogreTransform = ogre->AddComponent< TransformComponent>();;
    ogreTransform->SetWorldPosition(position);
    ogreTransform->SetWorldScale(2.0f);


    ModelComponent* modelComponent = ogre->AddComponent<ModelComponent>();
    modelComponent->SetModel(this->ogreModel);

    AnimatorComponent* animator = ogre->AddComponent<AnimatorComponent>();
    animator->AddAnimation("run", ogreModel->GetAnimations());
    animator->SetSkeletonVisible(true);
    animator->SetTime(static_cast<float>(rand() % 100));
    animator->Play("run", true);


    Health* ogreHealth = ogre->AddComponent<Health>();
    ogreHealth->SetMaxHealth(this->ogreHealth);
    ogreHealth->SetCurrentHealth(this->ogreHealth);

    BoxCollisionComponent* ogreBC = ogre->AddComponent<BoxCollisionComponent>();
    ogreBC->SetExtents({ 1.7f, 1.7f, 0.4f }); // here
    ogreBC->SetLayer(CollisionLayer::ENEMY);
    ogreBC->SetCenterOffset({ 0.0f, 1.8f, 0.0f });

    HurtboxComponent* ogreHurtbox = ogre->AddComponent<HurtboxComponent>();

    EnemyControllerComponent* ogreController = ogre->AddComponent<EnemyControllerComponent>();
    ogreController->SetMoveSpeed(this->ogreMoveSpeed);

    if (ogreController)
    {
        ogreController->ResetKnockback();
    }


    auto* ogreSplatter = ogre->AddComponent<SplatterComponent>();
    auto* ogreSoundComponent = ogre->AddComponent<SoundComponent>();
    auto ogreHitSound = context.assetManager->Get<Sound>("assets/sounds/troll_hit_sound.sound");
    auto ogreFootstepSound = context.assetManager->Get<Sound>("assets/sounds/troll_footstep_sound.sound");
    if (ogreController)
    {
        ogreController->SetHitSound(ogreHitSound);
        ogreController->SetFootstepSound(ogreFootstepSound);
    }
    ogreSoundComponent->SetSound(ogreHitSound);



    activeOgre.push_back(ogre);
    return true;
}

void EnemySpawnerComponent::SpawnBloodExplosion(Scene *scene, XMFLOAT3 position)
{
    Entity *bloodExplosionEntity = scene->AddEntity("BloodExplosion");
    bloodExplosionEntity->AddComponent<TransformComponent>()->SetWorldPosition(position);

    ParticleSystemComponent *ps = bloodExplosionEntity->AddComponent<ParticleSystemComponent>();
    ps->SetSpawnRadius(RNG::Float(0.1f, 0.4f));
    ps->SetIsBurst(true);
    ps->SetIsSpread(true);
    ps->SetCount(20);
    ps->SetStartTint({ 0.35f, 0.03f, 0.03f });
    ps->SetEndTint({ 0.15f, 0.02f, 0.02f });
    ps->SetStartScale(0.8f);
    ps->SetEndScale(0.8f);
    ps->SetLifeTime(2.0f);

    ps->SetVelocity({ 4.0f, 4.0f, 4.0f });
    ps->SetAcceleration({ 0.0f, -9.82f, 0.0f });

    ps->SetIsAnimate(true);
    ps->SetAtlasTexture(this->atlasTexture);
    ps->SetAtlasSize({ 8.0f, 8.0f });

    bloodExplosionEntity->AddComponent<LifeTimeComponent>()->SetLifeTime(2.0f);
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
            if (auto* splatter = gib->GetComponent<SplatterComponent>()) {
                splatter->Reset();
            }
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
            if (auto* transform = e->GetComponent<TransformComponent>())
            {
                DirectX::XMFLOAT3 enemyPos = transform->GetWorldPosition();

                if (auto* splatter = e->GetComponent<SplatterComponent>())
                {
                    float bloodRadius = RNG::Float(this->deathBloodRadiusMin,
                        this->deathBloodRadiusMax);

                    splatter->PaintFloorAtPosition(enemyPos, bloodRadius);

                    const int extraSplats = RNG::Int(1, 2);
                    for (int s = 0; s < extraSplats; ++s)
                    {
                        float offsetAngle = RNG::Float(0.0f, DirectX::XM_2PI);
                        float offsetDist = RNG::Float(0.3f, bloodRadius * 0.4f);

                        DirectX::XMFLOAT3 splatPos = {
                            enemyPos.x + cosf(offsetAngle) * offsetDist,
                            enemyPos.y,
                            enemyPos.z + sinf(offsetAngle) * offsetDist
                        };

                        splatter->PaintFloorAtPosition(
                            splatPos,
                            RNG::Float(bloodRadius * 0.5f, bloodRadius * 0.8f));
                    }
                }

                SpawnGibsFromPool(context, enemyPos);

                XMFLOAT3 position = enemyPos;
                position.y += 1.4f;
                SpawnBloodExplosion(context.sceneManager->GetCurrentScene(), position);
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


    for (auto it = activeOgre.begin(); it != activeOgre.end(); )
    {
        Entity* e = *it;
        if (!e) {
            it = activeOgre.erase(it);
            continue;
        }

        auto* hp = e->GetComponent<Health>();
        if (hp && hp->IsDead())
        {
            if (auto* transform = e->GetComponent<TransformComponent>())
            {
                DirectX::XMFLOAT3 ogrePos = transform->GetWorldPosition();

                if (auto* splatter = e->GetComponent<SplatterComponent>())
                {
                    float bloodRadius = RNG::Float(this->deathBloodRadiusMin,
                        this->deathBloodRadiusMax);

                    splatter->PaintFloorAtPosition(ogrePos, bloodRadius + 1.4f);

                    const int extraSplats = RNG::Int(1, 2);
                    for (int s = 0; s < extraSplats; ++s)
                    {
                        float offsetAngle = RNG::Float(0.0f, DirectX::XM_2PI);
                        float offsetDist = RNG::Float(0.3f, bloodRadius * 0.4f);

                        DirectX::XMFLOAT3 splatPos = {
                            ogrePos.x + cosf(offsetAngle) * offsetDist,
                            ogrePos.y,
                            ogrePos.z + sinf(offsetAngle) * offsetDist
                        };

                        splatter->PaintFloorAtPosition(
                            splatPos,
                            RNG::Float(bloodRadius * 0.5f, bloodRadius * 0.8f));
                    }
                }

                SpawnGibsFromPool(context, ogrePos, 6);

                XMFLOAT3 position = ogrePos;
                position.y += 2.5f;
                SpawnBloodExplosion(context.sceneManager->GetCurrentScene(), position);
            }

            this->GetOwner()->GetScene()->DestroyEntity(e);

            it = activeOgre.erase(it);
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
	this->gibModel[0] = context.assetManager->Get<Model>("assets/models/gib1.glb");
	this->gibModel[1] = context.assetManager->Get<Model>("assets/models/gib2.glb");
	this->gibModel[2] = context.assetManager->Get<Model>("assets/models/gib3.glb");
	this->gibModel[3] = context.assetManager->Get<Model>("assets/models/gib4.glb");

	this->enemyModel = context.assetManager->Get<Model>("assets/models/goblin.glb");

	this->atlasTexture = context.assetManager->Get<Texture2D>("assets/images/detailed_smoke_spritesheet.png");

    this->ogreModel = context.assetManager->Get<Model>("assets/models/goblin.glb");

	this->InitializeGibPool(context, this->GetOwner()->GetScene());
    this->InitializeEnemyPool(context, this->GetOwner()->GetScene());

    this->BeginWave();
}

void EnemySpawnerComponent::Update(const AppContext& context, float deltaTime)
{
    this->CleanupDeadEnemies(context);
    this->CleanGibs();

    // ---------- Goblins ----------
    if (this->goblinElapsed < this->enemySpawnDuration
        && this->goblinsSpawned < this->maxEnemiesAlive)
    {
        this->goblinElapsed += deltaTime;
        this->goblinTimer -= deltaTime;

        if (this->goblinTimer <= 0.0f)
        {
            this->goblinTimer += (this->enemySpawnDuration / this->maxEnemiesAlive);

            if (this->SpawnOneEnemyFromPool(context))
                this->goblinsSpawned++;
        }
    }

    // ---------- Ogres ----------
    if (this->ogreElapsed < this->ogreSpawnDuration
        && this->ogresSpawned < this->maxOgreEnemies)
    {
        this->ogreElapsed += deltaTime;
        this->ogreTimer -= deltaTime;

        if (this->ogreTimer <= 0.0f)
        {
            this->ogreTimer += (this->ogreSpawnDuration / this->maxOgreEnemies);

            if (this->SpawnOneOgre(context))
                this->ogresSpawned++;
        }
    }
}

void EnemySpawnerComponent::OnDestroy(const AppContext& context)
{
	for (size_t i = 0; i < 4; i++)
	{
		gibModel[i].reset();
	}

	this->enemyModel.reset();
}

void EnemySpawnerComponent::BeginWave()
{
    this->goblinTimer = (this->maxEnemiesAlive > 0)
        ? this->enemySpawnDuration / this->maxEnemiesAlive
        : 0.0f;
    this->goblinElapsed = 0.0f;
    this->goblinsSpawned = 0;

    this->ogreTimer = (this->maxOgreEnemies > 0)
        ? this->ogreSpawnDuration / this->maxOgreEnemies
        : 0.0f;
    this->ogreElapsed = 0.0f;
    this->ogresSpawned = 0;
}
