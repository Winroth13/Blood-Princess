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

    modelEntity->AddComponent<ParticleSystemComponent>();

    ParticleSystemComponent *particleSystemComponent = modelEntity->GetComponent<ParticleSystemComponent>();
    particleSystemComponent->SetAtlasTexture(appContext.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png"));
    particleSystemComponent->SetAtlasSize({ 12, 6 });
    particleSystemComponent->SetVelocity({ 0.0f, -1, 0.0f });
    particleSystemComponent->SetAcceleration({ 0.0f, 2.0, 0.0f });
    particleSystemComponent->SetLifeTime(2.0f);
    particleSystemComponent->SetStartScale(0.2f);
    particleSystemComponent->SetEndScale(0.4f);

    auto sfxClip = appContext.assetManager->Get<AudioClip>("assets/audio/music/looping.ogg");
    auto musicClip = appContext.assetManager->Get<AudioClip>("assets/audio/music/Test2.ogg");
    if (sfxClip) {
        AudioComponent* audioComponent = modelEntity->AddComponent<AudioComponent>(sfxClip);
        MusicComponent* musicComponent = modelEntity->AddComponent<MusicComponent>(musicClip);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);

        audioComponent->Play();
    }

    playerEntity->AddComponent<PlayerControllerComponent>()->SetActive(true);
    auto playerCol = playerEntity->AddComponent<SphereCollisionComponent>();
    playerCol->SetLayer(CollisionLayer::PLAYER);
    playerCol->SetStatic(false);
    playerCol->SetRadius(0.5f);

    Entity *lightEntity = outScene.AddEntity("light");
    lightEntity->AddComponent<AmbientLightComponent>();
    lightEntity->AddComponent<DirectionalLightComponent>()->SetIntensity(3.0f);

    Entity* EntitywallEntity = outScene.AddEntity("wall");
    auto wallTransform = EntitywallEntity->AddComponent<TransformComponent>();
    wallTransform->SetWorldPosition({ 0.0f, 0.0f, 3.0f });

    TransformComponent *transform = lightEntity->AddComponent<TransformComponent>();
    transform->SetWorldPosition(0.36f, 0.05f, 0.31f);

    ModelComponent* modelWallComponent = EntitywallEntity->AddComponent<ModelComponent>();
    modelWallComponent->SetModel(appContext.assetManager->Get<Model>("assets/models/wall.glb"));
    modelWallComponent->SetActive(false);

    //auto modelCol = EntitywallEntity->AddComponent<BoxCollisionComponent>();
    //modelCol->SetLayer(CollisionLayer::ENVIRONMENT);
    //modelCol->SetStatic(true);
    //modelCol->SetExtents({ 0.5f, 2.5f, 5.0f }); 

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