#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "components/music_component.hpp"
#include "components/camera_component.hpp"
#include "core/app_context.hpp"
#include "assets/asset_manager.hpp"
#include "components/model_component.hpp"

static void TestScene(Scene &outScene, const AppContext &appContext) {
    Entity *playerEntity = outScene.AddEntity("player");
    playerEntity->AddComponent<TransformComponent>();
    playerEntity->AddComponent<CameraComponent>();
    playerEntity->AddComponent<MusicComponent>(appContext.assetManager->Get<AudioClip>("Test1.ogg"));

    Entity *modelEntity = outScene.AddEntity("model");
    modelEntity->AddComponent<TransformComponent>();
    ModelComponent *modelComponent = modelEntity->AddComponent<ModelComponent>();
    modelComponent->SetModel(appContext.assetManager->Get<Model>("assets/princess.glb"));

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