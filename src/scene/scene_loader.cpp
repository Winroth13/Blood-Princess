#include "scene_loader.hpp"
#include "core/logging.hpp"
#include <cmath>
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "components/music_component.hpp"
#include "components/camera_component.hpp"
#include "core/app_context.hpp"
#include "assets/asset_manager.hpp"
#include "components/model_component.hpp"
#include "components/audio_component.hpp"
#include "components/listener_component.hpp"
#include "components/point_light_component.hpp"
#include "components/directional_light_component.hpp"
#include "components/spot_light_component.hpp"
#include "components/player_controller_component.hpp"
#include "components/box_collision_component.hpp"
#include "components/ambient_light_component.hpp"
#include "components/particle_system_component.hpp"
#include "components/sphere_collision_component.hpp"

static void TestScene(Scene &outScene, const AppContext &appContext) {
    Entity *playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->AddComponent<CameraComponent>();
    playerEntity->AddComponent<ListenerComponent>();
    playerEntity->GetComponent<TransformComponent>()->SetWorldPosition({ 0.0f, 0.0f, 0.0f });
    playerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/dashAction.ogg"));

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 0.0f, 0.0f, 0.0f });
    ModelComponent *modelComponent = modelEntity->AddComponent<ModelComponent>();
    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/falcata_sword.glb"));

    playerEntity->AddComponent<PlayerControllerComponent>()->SetActive(true);
    auto playerCol = playerEntity->AddComponent<SphereCollisionComponent>();
    playerCol->SetLayer(CollisionLayer::PLAYER);
    playerCol->SetStatic(false);
    playerCol->SetRadius(0.5f);

    Entity *lightEntity = outScene.AddEntity("light");
    lightEntity->AddComponent<AmbientLightComponent>();
    lightEntity->AddComponent<DirectionalLightComponent>()->SetIntensity(3.0f);
    TransformComponent *transform = lightEntity->AddComponent<TransformComponent>();
    transform->SetWorldPosition(0.0f, 0.0f, 0.0f);


    SceneLoader::CreateCircularRoom(outScene, appContext, { 0.0f, 0.0f, 0.0f }, 16);

    {
        Entity *entity = outScene.AddEntity("animation_entity");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(0.0f, -1.0f, 5.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/animations/demo_animation.glb"));
    }

    {
        Entity *entity = outScene.AddEntity("princess_entity");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(-3.0f, -1.0f, 5.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/princess.glb"));
    }

    {
        Entity *entity = outScene.AddEntity("sword_entity");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(-6.0f, 1.0f, 5.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/sword.glb"));
    }

    //Entity *animatedEntity = outScene.AddEntity("animated_entity");
    //animatedEntity->AddComponent<TransformComponent>()->SetWorldPosition(0.0f, 0.0f, 0.0f);
    //ModelComponent *animatedEntityModel = animatedEntity->AddComponent<ModelComponent>();
    //animatedEntityModel->SetModel(appContext.assetManager->Get<Model>("assets/models/animations/demo_animation.glb"));

    outScene.PrintEntityHierarchy();
}

void SceneLoader::CreateCircularRoom(Scene &outScene, const AppContext &appContext, const DirectX::XMFLOAT3 &center, int numSegments)
{
    if (numSegments <= 0)
    {
        return;
    }

    auto wallModel = appContext.assetManager->Get<Model>("assets/models/wall.glb");
    auto torchModel = appContext.assetManager->Get<Model>("assets/models/torch.glb");
    auto floorModel = appContext.assetManager->Get<Model>("assets/models/floor.glb");
    auto ceilingModel = appContext.assetManager->Get<Model>("assets/models/ceiling.glb");
    auto flameTexture = appContext.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png");
    auto smokeTexture = appContext.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png");
    auto fireSound = appContext.assetManager->Get<AudioClip>("assets/audio/music/egomassive__fire.ogg"); 

    const float wallLength = 10.0f;
    const float halfWallLength = wallLength * 0.5f;
    const float deltaAngle = DirectX::XM_2PI / static_cast<float>(numSegments);
    const float halfAngle = deltaAngle * 0.5f;
    // Apothem to the inner face of the wall segments so adjacent inner corners meet seamlessly
    const float innerRadius = halfWallLength / tanf(halfAngle);
    // Wall center is offset by half the wall thickness (0.5f)
    const float wallRadius = innerRadius + 0.5f;

    // 1. Build walls (2 walls high) and torches
    for (int i = 0; i < numSegments; ++i)
    {
        float theta = static_cast<float>(i) * deltaAngle;
        float posX = center.x + wallRadius * cosf(theta);
        float posY = center.y;
        float posZ = center.z + wallRadius * sinf(theta);

        // Yaw in degrees such that local +X points inwards toward the center of the room
        float yaw = 180.0f - DirectX::XMConvertToDegrees(theta);

        // Lower wall segment entity
        std::string wallName = "Wall_" + std::to_string(i);
        Entity *wallEntity = outScene.AddEntity(wallName);

        TransformComponent *wallTransform = wallEntity->AddComponent<TransformComponent>();
        wallTransform->SetWorldPosition(posX, posY, posZ);
        wallTransform->SetWorldRotation(0.0f, yaw, 0.0f);

        BoxCollisionComponent *wallCol = wallEntity->AddComponent<BoxCollisionComponent>();
        wallCol->SetLayer(CollisionLayer::ENVIRONMENT);
        wallCol->SetStatic(true);
        wallCol->SetExtents({ 0.5f, 2.5f, halfWallLength });

        ModelComponent *wallModelComponent = wallEntity->AddComponent<ModelComponent>();
        wallModelComponent->SetModel(wallModel);

        // Upper wall segment entity stacked 1 wall higher directly on top
        std::string upperWallName = "Wall_Upper_" + std::to_string(i);
        Entity *upperWallEntity = outScene.AddEntity(upperWallName);
        upperWallEntity->SetParent(wallEntity);

        TransformComponent *upperWallTransform = upperWallEntity->AddComponent<TransformComponent>();
        upperWallTransform->SetLocalPosition(0.0f, 5.0f, 0.0f);
        upperWallTransform->SetLocalRotation(0.0f, 0.0f, 0.0f);

        BoxCollisionComponent *upperWallCol = upperWallEntity->AddComponent<BoxCollisionComponent>();
        upperWallCol->SetLayer(CollisionLayer::ENVIRONMENT);
        upperWallCol->SetStatic(true);
        upperWallCol->SetExtents({ 0.5f, 2.5f, halfWallLength });

        ModelComponent *upperWallModelComponent = upperWallEntity->AddComponent<ModelComponent>();
        upperWallModelComponent->SetModel(wallModel);

        // Torch entity attached to the lower wall
        std::string torchName = "Torch_" + std::to_string(i);
        Entity *torchEntity = outScene.AddEntity(torchName);
        torchEntity->SetParent(wallEntity);

        TransformComponent *torchTransform = torchEntity->AddComponent<TransformComponent>();
        // Positioned on the inner face of the wall segment at eye level, offset slightly outward so the torch holder is clearly visible
        torchTransform->SetLocalPosition(0.58f, 0.0f, 0.0f);
        torchTransform->SetLocalRotation(0.0f, 0.0f, 0.0f);

        ModelComponent *torchModelComponent = torchEntity->AddComponent<ModelComponent>();
        torchModelComponent->SetModel(torchModel);

        // Fire particle entity attached to the torch
        std::string fireName = "FireParticle_" + std::to_string(i);
        Entity *fireParticleEntity = outScene.AddEntity(fireName);
        fireParticleEntity->SetParent(torchEntity);

        TransformComponent *fireTransform = fireParticleEntity->AddComponent<TransformComponent>();
        fireTransform->SetLocalPosition(0.17f, 0.3f, 0.0f);

        // Flame particle system
        ParticleSystemComponent *flame = fireParticleEntity->AddComponent<ParticleSystemComponent>();
        flame->SetAtlasTexture(flameTexture);
        flame->SetAtlasSize({ 12.0f, 6.0f });
        flame->SetVelocity({ 0.0f, 0.0f, 0.0f });
        flame->SetAcceleration({ 0.0f, 0.0f, 0.0f });
        flame->SetLifeTime(2.0f);
        flame->SetStartScale(0.3f);
        flame->SetEndScale(0.3f);
        flame->SetIsAnimate(true);
        flame->SetIsAdditive(true);
        flame->SetSpawnRadius(0.01f);
        flame->SetAnimationSpeed(1.2f);
        flame->SetCount(2);

        // Smoke particle system
        ParticleSystemComponent *smoke = fireParticleEntity->AddComponent<ParticleSystemComponent>();
        smoke->SetAtlasTexture(smokeTexture);
        smoke->SetAtlasSize({ 13.0f, 1.0f });
        smoke->SetVelocity({ 0.0f, 0.1f, 0.0f });
        smoke->SetAcceleration({ 0.0f, 0.0f, 0.0f });
        smoke->SetLifeTime(5.0f);
        smoke->SetStartScale(0.1f);
        smoke->SetEndScale(0.25f);
        smoke->SetIsAnimate(true);
        smoke->SetIsAdditive(true);
        smoke->SetSpawnRadius(0.08f);
        smoke->SetCount(25);
        smoke->SetStartTint({ 88.0f / 255.0f, 88.0f / 255.0f, 88.0f / 255.0f });
        smoke->SetEndTint({ 0.0f, 0.0f, 0.0f });

        // Point light on torch
        PointLightComponent *pointLight = fireParticleEntity->AddComponent<PointLightComponent>();
        pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });
        pointLight->SetIntensity(8.0f);
		pointLight->SetRadius(6.7f);

        // 3D Fire audio
        AudioComponent *audioComponent = fireParticleEntity->AddComponent<AudioComponent>(fireSound);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);
        audioComponent->SetAudioRadius(4.0f);
        audioComponent->SetVolume(50.0f);
        audioComponent->SetPlayOnStart(true);
    }

    // 2. Build floor and ceiling covering the area within the circumference of the wall circle
    const float tileSize = 10.0f;
    const float halfTile = tileSize * 0.5f;
    // Floor top surface is flush with the bottom of the lower walls (Y = center.y - 2.5f)
    const float floorY = center.y - 3.0f;
    // Ceiling bottom surface is flush with the top of the upper walls (Y = center.y + 7.5f)
    const float ceilingY = center.y + 8.0f;

    Entity *floorRoot = outScene.AddEntity("Floor");
    floorRoot->AddComponent<TransformComponent>()->SetWorldPosition(center.x, floorY, center.z);

    Entity *ceilingRoot = outScene.AddEntity("Ceiling");
    ceilingRoot->AddComponent<TransformComponent>()->SetWorldPosition(center.x, ceilingY, center.z);

    int tileIndex = 0;
    for (float x = -30.0f; x <= 30.0f; x += tileSize)
    {
        for (float z = -30.0f; z <= 30.0f; z += tileSize)
        {
            float closestX = (fabsf(x) <= halfTile) ? 0.0f : (fabsf(x) - halfTile);
            float closestZ = (fabsf(z) <= halfTile) ? 0.0f : (fabsf(z) - halfTile);
            float distSq = closestX * closestX + closestZ * closestZ;

            if (distSq < wallRadius * wallRadius)
            {
                // Floor tile
                std::string floorTileName = "FloorTile_" + std::to_string(tileIndex);
                Entity *floorTile = outScene.AddEntity(floorTileName);
                floorTile->SetParent(floorRoot);

                TransformComponent *floorTransform = floorTile->AddComponent<TransformComponent>();
                floorTransform->SetLocalPosition(x, 0.0f, z);

                BoxCollisionComponent *floorCol = floorTile->AddComponent<BoxCollisionComponent>();
                floorCol->SetLayer(CollisionLayer::ENVIRONMENT);
                floorCol->SetStatic(true);
                floorCol->SetExtents({ halfTile, 0.5f, halfTile });

                ModelComponent *floorComp = floorTile->AddComponent<ModelComponent>();
                floorComp->SetModel(floorModel);

                // Ceiling tile
                std::string ceilingTileName = "CeilingTile_" + std::to_string(tileIndex);
                Entity *ceilingTile = outScene.AddEntity(ceilingTileName);
                ceilingTile->SetParent(ceilingRoot);

                TransformComponent *ceilingTransform = ceilingTile->AddComponent<TransformComponent>();
                ceilingTransform->SetLocalPosition(x, 0.0f, z);

                BoxCollisionComponent *ceilingCol = ceilingTile->AddComponent<BoxCollisionComponent>();
                ceilingCol->SetLayer(CollisionLayer::ENVIRONMENT);
                ceilingCol->SetStatic(true);
                ceilingCol->SetExtents({ halfTile, 0.5f, halfTile });

                ModelComponent *ceilingComp = ceilingTile->AddComponent<ModelComponent>();
                ceilingComp->SetModel(ceilingModel);

                tileIndex++;
            }
        }
    }
}

bool SceneLoader::Initialize(const AppContext &appContext)
{
    LogInfo("Creating Scene Loader...");
    LogIndent();

    this->appContext = &appContext;

    LogUnindent();

    return true;
}

bool SceneLoader::Load(const std::string &name, Scene &outScene) {
    TestScene(outScene, *this->appContext);

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}