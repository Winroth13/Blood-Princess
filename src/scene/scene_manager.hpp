#pragma once

#include "scene/scene.hpp"
#include "scene/scene_loader.hpp"
#include "core/app_context.hpp"

#include <string>

class SceneManager
{
    SceneLoader loader;

	Scene scene;
    std::string currentSceneName;
    std::string targetSceneName;
    bool shouldReload = false;

    void ChangeScene(const std::string &name);

public:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager &other) = delete;
    SceneManager &operator=(const SceneManager &other) = delete;

    bool Initialize(const AppContext &context, const std::string &startSceneName);
    void Shutdown();

    void RequestSceneChange(const std::string &name);
    void ReloadCurrentScene();

    void Update(float deltaTime);

    Scene *GetCurrentScene() { return &this->scene; }
    const std::string &GetCurrentSceneName() const { return this->currentSceneName; }
};