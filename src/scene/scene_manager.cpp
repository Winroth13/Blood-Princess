#include "scene_manager.hpp"
#include "core/logging.hpp"

void SceneManager::ChangeScene(const std::string &name) {
    if (!this->loader.Load(name, this->scene)) {
        LogWarn("Failed to load scene '%s'", name.c_str());
        return;
    }

    this->currentSceneName = name;
    this->shouldReload = false;
}

bool SceneManager::Initialize(const AppContext &context, const std::string &startSceneName) {
    LogInfo("Creating Scene Manager...");
    LogIndent();

    this->scene.SetAppContext(context);

    this->RequestSceneChange(startSceneName);

    LogUnindent();
    return true;
}

void SceneManager::Shutdown() {
    this->scene.Clear();
}

void SceneManager::RequestSceneChange(const std::string &name) {
    this->targetSceneName = name;
}

void SceneManager::ReloadCurrentScene() {
    this->shouldReload = true;
}

void SceneManager::Update(float deltaTime) {
    if (this->targetSceneName != this->currentSceneName || this->shouldReload) {
        this->ChangeScene(this->targetSceneName);
        this->shouldReload = false;
    }

    this->scene.Update(deltaTime);
}
