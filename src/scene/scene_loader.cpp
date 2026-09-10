#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform.hpp"
#include "components/music.hpp"
#include "core/app_context.hpp";
#include "assets/asset_manager.hpp"

static void TestScene(Scene &outScene, const AppContext &appContext) {
    Entity *testParent = outScene.AddEntity("test_parent");
    testParent->AddComponent<Transform>();

    Entity *testChild = outScene.AddEntity("test_child");
    testChild->AddComponent<Transform>();

    testChild->SetParent(testParent);

    Entity *testChild2 = outScene.AddEntity("test_child_2");
    testChild2->AddComponent<Transform>();

    Entity *testParent2 = outScene.AddEntity("test_parent_2");
    testParent2->AddComponent<Transform>();
    
    auto music = appContext.assetManager->Get<AudioClip>("Test1.ogg");
    testParent2->AddComponent<Music>(music);
   
    testChild2->SetParent(testParent2);

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