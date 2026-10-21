#include "scene_manager.hpp"
#include "core/logging.hpp"
#include "editor/editor.hpp"

#include "scene/scenes/scene_main_menu.hpp"
#include "scene/scenes/scene_gameplay.hpp"
#include "scene/scenes/scene_death_screen.hpp"
#include "scene/scenes/scene_win_screen.hpp"
#include "scene/scenes/scene_settings.hpp"
#include "scene/scenes/scene_level_transition.hpp"

void SceneManager::ChangeScene(const std::string &name) {
    if (!this->loader.Load(name, this->scene)) {
        LogWarn("Failed to load scene '%s'", name.c_str());
        return;
    }

    this->scene.GetAppContext().editor->SetShowEditor(false); // lol

    this->currentSceneName = name;
    this->shouldReload = false;
}

void SceneManager::RegisterScenes() {
    this->registry.AddScene("main_menu", Scenes::MainMenu);
    this->registry.AddScene("gameplay", Scenes::Gameplay);
    this->registry.AddScene("level_transition", Scenes::LevelTransition);
    this->registry.AddScene("death_screen", Scenes::DeathScreen);
    this->registry.AddScene("win_screen", Scenes::WinScreen);
    this->registry.AddScene("settings", Scenes::Settings);
}

bool SceneManager::Initialize(const AppContext &context, const std::string &startSceneName) {
    LogInfo("Creating Scene Manager...");
    LogIndent();

    if (!this->loader.Initialize(context, &this->registry))
    {
        LogError("Failed to create Scene Loader");
        return false;
    }

    this->scene.SetAppContext(context);

    this->RegisterScenes();

    this->RequestSceneChange(startSceneName);

    LogUnindent();
    return true;
}

void SceneManager::Shutdown() {
    this->scene.Clear();
    this->registry.Clear();
}

void SceneManager::RequestSceneChange(const std::string &name) {
    this->targetSceneName = name;
}

void SceneManager::ReloadCurrentScene() {
    this->shouldReload = true;
}

void SceneManager::Update(float deltaTime, float fixedDeltaTime, int numFixedTicks) {
    if (this->targetSceneName != this->currentSceneName || this->shouldReload) {
        this->ChangeScene(this->targetSceneName);
        this->shouldReload = false;
    }

    this->scene.Update(deltaTime, fixedDeltaTime, numFixedTicks);
}