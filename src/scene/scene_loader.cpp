#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform.hpp"

bool SceneLoader::Load(const std::string &name, Scene &outScene) {
    Entity *testEntity = outScene.AddEntity("test_entity");
    testEntity->AddComponent<Transform>();

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}