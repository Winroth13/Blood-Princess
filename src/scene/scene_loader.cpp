#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform.hpp"

static void TestScene(Scene &outScene) {
    Entity *testParent = outScene.AddEntity("test_parent");
    testParent->AddComponent<Transform>();

    Entity *testChild = outScene.AddEntity("test_child");
    testChild->AddComponent<Transform>();

    testChild->SetParent(testParent);

    Entity *testChild2 = outScene.AddEntity("test_child_2");
    testChild2->AddComponent<Transform>();

    Entity *testParent2 = outScene.AddEntity("test_parent_2");
    testParent2->AddComponent<Transform>();

    testChild2->SetParent(testParent2);

    outScene.PrintEntityHierarchy();
}

bool SceneLoader::Load(const std::string &name, Scene &outScene) {
    TestScene(outScene);

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}