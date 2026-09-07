#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform.hpp"

static void TestScene(Scene &outScene) {
    Entity *testEntity = outScene.AddEntity("test_entity");
    testEntity->AddComponent<Transform>();
}

bool SceneLoader::Load(const std::string &name, Scene &outScene) {
    TestScene(outScene);

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}