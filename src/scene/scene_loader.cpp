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

static void TestScene(Scene &outScene, const AppContext &appContext) {
    Entity *playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->AddComponent<CameraComponent>();
    //playerEntity->AddComponent<MusicComponent>(appContext.assetManager->Get<AudioClip>("Test1.ogg"));

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>();
    TransformComponent *transformComponent = modelEntity->GetComponent<TransformComponent>();
    modelEntity->SetParent(playerEntity);

    ModelComponent *modelComponent = modelEntity->AddComponent<ModelComponent>();
    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/princess.glb"));

    auto sfxClip = appContext.assetManager->Get<AudioClip>("Test2.ogg");
    if (sfxClip) {
        AudioComponent* audioComponent = modelEntity->AddComponent<AudioComponent>(sfxClip);
        modelEntity->GetComponent<TransformComponent>()->SetWorldPosition({10,0,0});
        audioComponent->SetLooping(false);
        audioComponent->SetAudio3D(true);



        playerEntity->GetComponent<TransformComponent>()->SetWorldPosition({0,0,0});
        playerEntity->AddComponent<ListenerComponent>();

        // Components' OnStart() is invoked by Scene::Update via StartPendingEntities().
        // Call Update(0.0f) to initialize components now so Play() can use the cached AudioManager.
        outScene.Update(1.0f);

        audioComponent->Play();
    }

    for (int i = 0; i < 600; ++i) {
        float x = 0.0f + i * 1.0f; // move player along +X toward source
        playerEntity->GetComponent<TransformComponent>()->SetLocalPosition({ 600, 600, 0.0f });
        outScene.Update(1.0f / 60.0f); // simulate a frame
    }
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