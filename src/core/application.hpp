#pragma once

#include "core/window.hpp"
#include "rendering/renderer.hpp"
#include "assets/asset_manager.hpp"
#include "scene/scene_manager.hpp"
#include "audio/audio_manager.hpp"
#include "editor/imgui_layer.hpp"
#include "editor/editor.hpp"
#include "collision/collision_manager.hpp"

#include <chrono>

struct AppContext;

class Application 
{
	Window window;
	Renderer renderer;
	AssetManager assetManager;
	SceneManager sceneManager;
	AudioManager audioManager;
    ImGuiLayer imGuiLayer;
    Editor editor;
    CollisionManager collisionManager;

	bool isRunning = false;

    const AppContext appContext = {
        this,
        &this->window,
        &this->renderer,
        &this->assetManager,
        &this->sceneManager,
        &this->audioManager,
        &this->imGuiLayer,
        &this->editor,
        &this->collisionManager
    };

    void Update(float deltaTime);
    void Render(float deltaTime);

public:
    Application() = default;
    ~Application() = default;

    Application(const Application &other) = delete;
    Application &operator=(const Application &other) = delete;

	bool Initialize();
	void Run();
	void Shutdown();
};