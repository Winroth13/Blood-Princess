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

static void TestScene(Scene &outScene, const AppContext &appContext) {
    Entity *playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->AddComponent<CameraComponent>();
    playerEntity->AddComponent<ListenerComponent>();
    playerEntity->GetComponent<TransformComponent>()->SetWorldPosition({ 0.0f, 0.0f, 0.0f });

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>();
    modelEntity->GetComponent<TransformComponent>()->SetWorldPosition({ 0.0f, 0.0f, 3.0f });

    ModelComponent *modelComponent = modelEntity->AddComponent<ModelComponent>();
    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/monkey.glb"));

    auto sfxClip = appContext.assetManager->Get<AudioClip>("looping.ogg");
    auto musicClip = appContext.assetManager->Get<AudioClip>("Test1.ogg");
    if (sfxClip) {
        AudioComponent* audioComponent = modelEntity->AddComponent<AudioComponent>(sfxClip);
        MusicComponent* musicComponent = modelEntity->AddComponent<MusicComponent>(musicClip);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);
        outScene.Update(0.0f);

        audioComponent->Play();
    }
    playerEntity->AddComponent<PointLightComponent>();
    playerEntity->AddComponent<DirectionalLightComponent>();
    playerEntity->AddComponent<SpotLightComponent>();
    playerEntity->AddComponent<PlayerControllerComponent>();
    playerEntity->AddComponent<BoxCollisionComponent>();
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