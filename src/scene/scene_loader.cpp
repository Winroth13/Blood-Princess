#include "scene_loader.hpp"
#include "core/logging.hpp"
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

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 0.13f, -0.12f, 0.4f });
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
    transform->SetWorldPosition(0.36f, 0.05f, 0.31f);


    Entity* EntitywallEntity = outScene.AddEntity("wall");
    auto wallTransform = EntitywallEntity->AddComponent<TransformComponent>();
    wallTransform->SetWorldPosition({ 5.0f, 0.0f, 0.0f });
    auto modelCol = EntitywallEntity->AddComponent<BoxCollisionComponent>();
    modelCol->SetLayer(CollisionLayer::ENVIRONMENT);
    modelCol->SetStatic(true);
    modelCol->SetExtents({ 0.5f, 2.5f, 5.0f }); 
    ModelComponent* modelWallComponent = EntitywallEntity->AddComponent<ModelComponent>();
    modelWallComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/wall.glb"));


    {
        Entity *torchEntity = outScene.AddEntity("Torch");
        torchEntity->AddComponent<TransformComponent>();
        torchEntity->GetComponent<TransformComponent>()->SetLocalPosition(1, 0, 0);

        torchEntity->AddComponent<ModelComponent>()->SetModel(appContext.assetManager->Get<Model>("assets/models/torch.glb"));

        Entity *fireParticleEntity = outScene.AddEntity("Fire Particle");
        fireParticleEntity->AddComponent<TransformComponent>()->SetLocalPosition(0.17f, 0.3f, 0.0f);
        fireParticleEntity->AddComponent<ParticleSystemComponent>();

        ParticleSystemComponent *particleSystemComponent = fireParticleEntity->GetComponent<ParticleSystemComponent>();
        particleSystemComponent->SetAtlasTexture(appContext.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png"));
        particleSystemComponent->SetAtlasSize({ 12, 6 });
        particleSystemComponent->SetVelocity({0, 0, 0});
        particleSystemComponent->SetAcceleration({ 0, 0, 0 });
        particleSystemComponent->SetLifeTime(2.0f);
        particleSystemComponent->SetStartScale(0.3f);
        particleSystemComponent->SetEndScale(0.3f);
        particleSystemComponent->SetIsAnimate(true);
        particleSystemComponent->SetIsAdditive(true);
        particleSystemComponent->SetSpawnRadius(0.01f);
        particleSystemComponent->SetAnimationSpeed(1.2f);
        particleSystemComponent->SetCount(2);

        fireParticleEntity->SetParent(torchEntity);

        ParticleSystemComponent *smoke = fireParticleEntity->AddComponent<ParticleSystemComponent>();
        smoke->SetAtlasTexture(appContext.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png"));
        smoke->SetAtlasSize({ 13, 1 });
        smoke->SetVelocity({ 0, 0.1f, 0 });
        smoke->SetAcceleration({ 0, 0, 0 });
        smoke->SetLifeTime(5.0f);
        smoke->SetStartScale(0.1f);
        smoke->SetEndScale(0.25f);
        smoke->SetIsAnimate(true);
        smoke->SetIsAdditive(true);
        smoke->SetSpawnRadius(0.08f);
        smoke->SetCount(25);
        smoke->SetStartTint({ 88 / 255.0f, 88 / 255.0f ,88 / 255.0f });
        smoke->SetEndTint({ 0, 0, 0 });

        fireParticleEntity->AddComponent<PointLightComponent>();
        auto pointLightC = fireParticleEntity->GetComponent<PointLightComponent>();
        pointLightC->SetColour({ 228 / 255.0f, 119 / 255.0f, 37 / 255.0f });
        pointLightC->SetIntensity(8);

        auto sfxClip = appContext.assetManager->Get<AudioClip>("assets/audio/music/egomassive__fire.ogg");
        AudioComponent* audioComponent = fireParticleEntity->AddComponent<AudioComponent>(sfxClip);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);
        audioComponent->SetAudioRadius(2.0f);
        audioComponent->SetPlayOnStart(true);

    }

    //Entity *animatedEntity = outScene.AddEntity("animated_entity");
    //animatedEntity->AddComponent<TransformComponent>()->SetWorldPosition(0.0f, 0.0f, 0.0f);
    //ModelComponent *animatedEntityModel = animatedEntity->AddComponent<ModelComponent>();
    //animatedEntityModel->SetModel(appContext.assetManager->Get<Model>("assets/models/animations/demo_animation.glb"));

    outScene.PrintEntityHierarchy();
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