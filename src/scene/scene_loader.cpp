#include "scene_loader.hpp"
#include "core/logging.hpp"
#include "scene/scene_registry.hpp"

bool SceneLoader::Initialize(const AppContext &appContext, SceneRegistry *registry) {
    LogInfo("Creating Scene Loader...");
    LogIndent();

    if (!registry) {
        LogError("Registry was nullptr");
        return false;
    }

    this->appContext = &appContext;
    this->registry = registry;

    LogUnindent();

    return true;
}

bool SceneLoader::Load(const std::string &name, Scene &outScene) {
    std::function<void(Scene &, const AppContext &)> *func = this->registry->GetScene(name);
    if (!func) {
        LogWarn("Scene '%s' has not been registered", name.c_str());
        return false;
    }

    outScene.Clear();
    (*func)(outScene, *this->appContext);
    outScene.SetName(name);

    LogInfo("Loaded scene '%s'", name.c_str());

    return true;
}