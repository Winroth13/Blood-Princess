#include "scene_loader.hpp"
#include "core/logging.hpp"
#include <cmath>
#include "scene/entity.hpp"
#include "components/core/transform_component.hpp"
#include "components/audio/music_component.hpp"
#include "components/core/camera_component.hpp"
#include "core/app_context.hpp"
#include "assets/asset_manager.hpp"
#include "components/core/model_component.hpp"
#include "components/audio/audio_component.hpp"
#include "components/audio/listener_component.hpp"
#include "components/lights/point_light_component.hpp"
#include "components/lights/directional_light_component.hpp"
#include "components/lights/spot_light_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/lights/ambient_light_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
#include "components/lights/light_flicker_component.hpp"
#include "components/skeletal/animator_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/ai/enemy_controller_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "components/skeletal/socket_component.hpp"
#include "components/audio/character_audio_component.hpp"

void CreatePlayer(Scene &outScene, const AppContext &appContext)
{
    Entity* playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<PlayerControllerComponent>()->SetActive(true);
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->GetComponent<TransformComponent>()->SetWorldPosition({ 0.0f, -1.0f, 0.0f });
    playerEntity->AddComponent<ListenerComponent>();

    /* Audio */
    {
        playerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/dashAction.ogg"));
        auto characterAudio = playerEntity->AddComponent<CharacterAudioComponent>();
        characterAudio->SetIsPlayer(true);

		SoundLayer swingLayer;
		swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/slash_attack_1.ogg"), 0.9f, 1.1f, 0.95f, 1.05f);
		swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/slash_attack_2.ogg"), 0.9f, 1.1f, 0.95f, 1.05f);
		swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/slash_attack_3.ogg"), 0.9f, 1.1f, 0.95f, 1.05f);
		swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/slash_attack_4.ogg"), 0.9f, 1.1f, 0.95f, 1.05f);
		characterAudio->AddLightAttackSwingLayer(swingLayer);

        SoundLayer squishLayer;
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/271666__honorhunter__tomato-squishwet.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/500912__bertsz__squish-impact.ogg"), 0.70f, 0.90f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/536765__egomassive__squish.ogg"), 0.70f, 0.90f, 0.93f, 1.07f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/686363__faircashew__squished.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/524952__magnuswaker__squelch-2.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/540076__magnuswaker__meat-slap-2.ogg"), 0.60f, 0.80f, 0.94f, 1.04f);

        SoundLayer boneLayer;
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/192146__brassknucklesfilms__bone-crunch-fast.ogg"), 0.45f, 0.60f, 0.94f, 1.06f);
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/369601__evanboyerman__bone-crunchcrack-2-mixed.ogg"), 0.45f, 0.60f, 0.93f, 1.05f);
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/869107__robo9418__bone-crack.ogg"), 0.45f, 0.60f, 0.95f, 1.05f);

        SoundLayer thudLayer;
        thudLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/339832__insanity54__thud.ogg"), 0.70f, 0.85f, 0.92f, 1.04f);

		characterAudio->AddLightAttackLayer(squishLayer);
		characterAudio->AddLightAttackLayer(boneLayer);
		characterAudio->AddLightAttackLayer(thudLayer);

        SoundLayer heavySwingLayer;
        heavySwingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/523230__magnuswaker__hard-swing-1.ogg"), 1.2f, 1.3f, 0.92f, 1.04f);

        SoundLayer heavyGoreLayer;
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/563491__magnuswaker__gore-rend.ogg"), 0.75f, 0.95f, 0.93f, 1.05f);
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/581092__magnuswaker__burst-flesh.ogg"), 0.75f, 0.95f, 0.93f, 1.05f);
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/563315__magnuswaker__gushing-flesh.ogg"), 0.70f, 0.90f, 0.95f, 1.05f);
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/522168__magnuswaker__gut-rip-bladed.ogg"), 0.75f, 0.95f, 0.92f, 1.04f);

        SoundLayer heavyBoneLayer;
        heavyBoneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/522171__magnuswaker__bone-breaker-1.ogg"), 0.65f, 0.80f, 0.92f, 1.03f);
        heavyBoneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/369601__evanboyerman__bone-crunchcrack-2-mixed.ogg"), 0.60f, 0.75f, 0.90f, 1.02f);

        SoundLayer heavyImpactLayer;
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/522091__magnuswaker__pound-of-flesh-1.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/528263__magnuswaker__pound-of-flesh-2.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/530117__magnuswaker__pound-of-flesh-3.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);

		characterAudio->AddHeavyAttackLayer(heavySwingLayer);
		characterAudio->AddHeavyAttackLayer(heavyGoreLayer);
		characterAudio->AddHeavyAttackLayer(heavyBoneLayer);
		characterAudio->AddHeavyAttackLayer(heavyImpactLayer);

        SoundLayer footstepsLayer;
		footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_1.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_2.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_3.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_4.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_5.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_6.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
		characterAudio->AddFootstepLayer(footstepsLayer);


    }

    /* Collisions */
    {
        auto playerCol = playerEntity->AddComponent<SphereCollisionComponent>();
        playerEntity->AddComponent<CameraComponent>();
        playerCol->SetLayer(CollisionLayer::PLAYER);
        playerCol->SetStatic(false);
        playerCol->SetRadius(0.5f);
    }

    /* Viewmodel */
    {
        Entity *viewEntity = outScene.AddEntity("viewmodel");
        auto *viewTransform = viewEntity->AddComponent<TransformComponent>();
        viewTransform->SetLocalPosition({ 0.0f, 0.0f, 0.0f });
        viewTransform->SetLocalScale({ 0.2f, 0.2f ,0.2f });
        viewTransform->SetLocalRotation({ 0.0f, 180.0f, 0.0f });
        ModelComponent *viewmodelModel = viewEntity->AddComponent<ModelComponent>();
        viewmodelModel->SetModel(appContext.assetManager->Get<Model>("assets/models/princess_viewmodel.glb"));

        playerEntity->GetComponent<PlayerControllerComponent>()->SetRootTransform(viewTransform);

        AnimatorComponent *animator = viewEntity->AddComponent<AnimatorComponent>();
        animator->AddAnimations(viewmodelModel->GetModel()->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("light_attack_01", true);

        auto *collider = viewEntity->AddComponent<BoxCollisionComponent>();
        collider->SetLayer(CollisionLayer::PLAYER_ATTACK);
        collider->SetTrigger(true);
        collider->SetExtents({ 1.0f, 0.5f, 1.5f });
        collider->SetCenterOffset({ 0.0f, 0.5, 1.0f });

        auto *swordHitBox = viewEntity->AddComponent<HitboxComponent>();
        swordHitBox->SetActive(false);
        swordHitBox->SetKnockbackForce(2);

        viewEntity->SetParent(playerEntity);

        viewEntity->AddComponent<CameraComponent>();
        auto camera = viewEntity->GetComponent<CameraComponent>();
        
        Entity *cameraEntity = outScene.AddEntity("Camera");
        cameraEntity->AddComponent<CameraComponent>();
        cameraEntity->AddComponent<TransformComponent>();
        cameraEntity->SetParent(viewEntity);

        SocketComponent *cameraSocket = cameraEntity->AddComponent<SocketComponent>();
        cameraSocket->SetJointName("head");
        cameraSocket->SetOffsetPosition({ 0.0, 0.0f, 0.0f });
        cameraSocket->SetOffsetRotation({ -90.0, 90.0f, 0.0f });
        cameraSocket->SetOffsetScale({ 1.0f, 1.0f, 1.0f });

        Entity *swordEntity = outScene.AddEntity("sword");
        auto *swordTransform = swordEntity->AddComponent<TransformComponent>();
        ModelComponent *swordModel = swordEntity->AddComponent<ModelComponent>();
        swordModel->SetModel(appContext.assetManager->Get<Model>("assets/models/sword.glb"));
        SocketComponent *socket = swordEntity->AddComponent<SocketComponent>();
        socket->SetJointName("socket.r");
        socket->SetOffsetPosition({ -1.2f, 0.0f, 0.5f });
        socket->SetOffsetRotation({ -90.0f, -90.0f, 12.0f });
        socket->SetOffsetScale({ 1.0f, 1.0f, 1.0f });
        swordEntity->SetParent(viewEntity);
    }
}

static void TestScene(Scene& outScene, const AppContext& appContext) {
    //Entity *musicEntity = outScene.AddEntity("music_entity");
    //musicEntity->AddComponent<TransformComponent>();
    //MusicComponent *musicComponent = musicEntity->AddComponent<MusicComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/Test2.ogg"));
    //musicComponent->SetVolume(0.2f);

    CreatePlayer(outScene, appContext);
    
    // Enemy Spawner
    Entity* spawnerEntity = outScene.AddEntity("Spawner");
    spawnerEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 0,-2,0 });
    spawnerEntity->AddComponent<EnemySpawnerComponent>();

    Entity *lightEntity = outScene.AddEntity("light");
    AmbientLightComponent *ambientLight = lightEntity->AddComponent<AmbientLightComponent>();
    ambientLight->SetColour({0.86f, 0.59f, 0.47f});
    ambientLight->SetIntensity(0.2f);
    lightEntity->AddComponent<DirectionalLightComponent>()->SetIntensity(3.0f);
    TransformComponent *transform = lightEntity->AddComponent<TransformComponent>();
    transform->SetWorldPosition(0.0f, 0.0f, 0.0f);
    transform->SetWorldRotation(30.0f, 0.0f, 0.0f);

    Entity *spotLightEntity = outScene.AddEntity("spot light");
    SpotLightComponent *spotLight = spotLightEntity->AddComponent<SpotLightComponent>();
    spotLight->SetIntensity(50.0f);
    spotLight->SetRange(30.0f);
    spotLight->SetColor({ 231.0f / 255.0f, 172.0f / 255.0f, 80.0f / 255.0f });
    spotLight->SetInnerAngle(XMConvertToRadians(60.0f));
    spotLight->SetOuterAngle(XMConvertToRadians(80.0f));
    TransformComponent *spotTransform = spotLightEntity->AddComponent<TransformComponent>();
    spotTransform->SetWorldPosition(-8.25f, -0.7f, 5.07f);
    spotTransform->SetWorldRotation(0, 116, 0);

    Entity *pointLightEntity = outScene.AddEntity("point light");
    PointLightComponent *pointLight = pointLightEntity->AddComponent<PointLightComponent>();
    pointLight->SetIntensity(220.0f);
    pointLight->SetRadius(10.0f);
    pointLight->SetCastShadows(false);
    TransformComponent *pointTransform = pointLightEntity->AddComponent<TransformComponent>();
    pointTransform->SetWorldPosition(-0.6f, -1.0f, 6.8f);

    SceneLoader::CreateCircularRoom(outScene, appContext, { 0.0f, 0.0f, 0.0f }, 16);

    //{
    //    Entity *entity = outScene.AddEntity("animation_entity");
    //    TransformComponent *transform = entity->AddComponent<TransformComponent>();
    //    transform->SetWorldPosition(0.0f, -1.0f, 5.0f);
    //    transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
    //    ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
    //    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/animations/demo_animation.glb"));
    //}

    {
        Entity *entity = outScene.AddEntity("princess");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(-3.0f, -2.5f, 5.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        transform->SetWorldScale(0.5f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/princess.glb"));
        auto* entityCollision = entity->AddComponent<BoxCollisionComponent>();
        entityCollision->SetExtents({ 1,1,1 });
        entityCollision->SetLayer(CollisionLayer::ENEMY);
        auto* entityHurtbox = entity->AddComponent<HurtboxComponent>();

        entity->AddComponent<Health>();
    }

    //{
    //    Entity *entity = outScene.AddEntity("sword_entity");
    //    TransformComponent *transform = entity->AddComponent<TransformComponent>();
    //    transform->SetWorldPosition(-6.0f, 1.0f, 5.0f);
    //    transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
    //    ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
    //    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/sword.glb"));
    //}

    {
        Entity *entity = outScene.AddEntity("animated_entity_twerk");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(-1.0f, -2.5f, 8.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        transform->SetWorldScale(0.5f, 0.5f, 0.5f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        std::shared_ptr<Model> model = appContext.assetManager->Get<Model>("assets/models/animations/princess_twerk.glb");
        modelComponent->SetModel(model);
        AnimatorComponent *animator = entity->AddComponent<AnimatorComponent>();
        animator->AddAnimation("twerk", model->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("twerk", true);
    }

    {
        Entity *entity = outScene.AddEntity("animated_entity_gangnam_style");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(1.0f, -2.5f, 8.0f);
        transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
        ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
        std::shared_ptr<Model> model = appContext.assetManager->Get<Model>("assets/models/animations/demo_gangnam_style.glb");
        modelComponent->SetModel(model);
        AnimatorComponent *animator = entity->AddComponent<AnimatorComponent>();
        animator->AddAnimation("gangnam_style", model->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("gangnam_style", true);
    }

    {
        Entity *entity = outScene.AddEntity("animation_light");
        TransformComponent *transform = entity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(0.0f, 0.5f, 8.0f);
        PointLightComponent *pointLight = entity->AddComponent<PointLightComponent>();
        pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });
        pointLight->SetRadius(8.0f);
        pointLight->SetIntensity(50.0f);
    }

    auto wallModel = appContext.assetManager->Get<Model>("assets/models/wall.glb");
    Entity *paintableWallEntity = outScene.AddEntity("paintable_wall");
    paintableWallEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 5.26f, 0.0f, 0.0f });
    paintableWallEntity->AddComponent<ModelComponent>()->SetModel(wallModel);
    paintableWallEntity->AddComponent<PaintableComponent>();

    PaintableComponent *paintableComponent = paintableWallEntity->GetComponent<PaintableComponent>();
    paintableComponent->SetPaintMask(appContext.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));
    paintableComponent->SetPaintMaskWidth(128);
    paintableComponent->SetPaintMaskHeight(128);

    paintableWallEntity->AddComponent<ParticleSystemComponent>();
    ParticleSystemComponent *psComponent = paintableWallEntity->GetComponent<ParticleSystemComponent>();
    psComponent->SetCount(16);
    psComponent->SetSpawnRadius(0.1f);
    psComponent->SetLifeTime(1.0f);

    psComponent->SetStartScale(1.2f);
    psComponent->SetEndScale(0.25f);

    psComponent->SetAtlasTexture(appContext.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));
    psComponent->SetIsAnimate(false);

    psComponent->SetVelocity({ 0.0f, 3.0f, 0.0f });
    psComponent->SetAcceleration({ 0.0, -9.82f, 0.0f });

    //outScene.PrintEntityHierarchy();
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
        wallModelComponent->SetTint({150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f});

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
        upperWallModelComponent->SetTint({150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f});

        // Torch entity attached to the lower wall
        std::string torchName = "Torch_" + std::to_string(i);
        Entity *torchEntity = outScene.AddEntity(torchName);
        torchEntity->SetParent(wallEntity);

        TransformComponent *torchTransform = torchEntity->AddComponent<TransformComponent>();
        // Positioned on the inner face of the wall segment at eye level, offset slightly outward so the torch holder is clearly visible
        torchTransform->SetLocalPosition(0.58f, 0.0f, 0.0f);
        torchTransform->SetLocalRotation(0.0f, 0.0f, 0.0f);
        torchTransform->SetLocalScale(2.0f, 2.0f, 2.0f);


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
        smoke->SetStartScale(0.2f);
        smoke->SetEndScale(0.5f);
        smoke->SetIsAnimate(true);
        smoke->SetIsAdditive(true);
        smoke->SetSpawnRadius(0.08f);
        smoke->SetCount(25);
        smoke->SetStartTint({ 150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f });
        smoke->SetEndTint({ 0.0f, 0.0f, 0.0f });

        // Point light on torch
        PointLightComponent *pointLight = fireParticleEntity->AddComponent<PointLightComponent>();
        pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });

		pointLight->SetRadius(8.0f);
        pointLight->SetIntensity(50.0f);

        fireParticleEntity->AddComponent<LightFlickerComponent>();

        // 3D Fire audio
        AudioComponent *audioComponent = fireParticleEntity->AddComponent<AudioComponent>(fireSound);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);
        audioComponent->SetMinDistance(3.0f);
        audioComponent->SetMaxDistance(15.0f);
        audioComponent->SetVolume(0.8f);
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

static void TestScene2(Scene &outScene, const AppContext &appContext) {
    Entity *playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->AddComponent<ListenerComponent>();
    playerEntity->GetComponent<TransformComponent>()->SetWorldPosition({ 0.0f, -1.0f, 0.0f });
    playerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/dashAction.ogg"));

    playerEntity->AddComponent<PlayerControllerComponent>()->SetActive(true);
    playerEntity->AddComponent<CameraComponent>();
    auto playerCol = playerEntity->AddComponent<SphereCollisionComponent>();
    playerCol->SetLayer(CollisionLayer::PLAYER);
    playerCol->SetStatic(false);
    playerCol->SetRadius(0.5f);

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 0.0f, 0.0f, 2.0f });
    ModelComponent *modelComponent = modelEntity->AddComponent<ModelComponent>();
    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/falcata_sword.glb"));
    modelEntity->SetParent(playerEntity);

    Entity *lightEntity = outScene.AddEntity("light");
    AmbientLightComponent *ambientLight = lightEntity->AddComponent<AmbientLightComponent>();
    ambientLight->SetColour({0.86f, 0.59f, 0.47f});
    ambientLight->SetIntensity(1.0f);
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
    outScene.Clear();
    TestScene(outScene, *this->appContext);

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}