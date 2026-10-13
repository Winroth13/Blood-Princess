#pragma once

#include "core/window.hpp"
#include "rendering/renderer.hpp"
#include "assets/asset_manager.hpp"
#include "scene/scene_manager.hpp"
#include "audio/audio_manager.hpp"
#include "editor/imgui_layer.hpp"
#include "editor/editor.hpp"
#include "collision/collision_manager.hpp"
#include "ai/ai_manager.hpp"
#include <chrono>

#define ENABLE_IMGUI_EDITOR 1

struct AppContext;

class Application 
{
    float totalTime = 0.0f;
    float fixedDeltaTime = 1.0f / 60.0f;
    float deltaTimeElapsed = 0.0f;

	Window window;
	Renderer renderer;
	AssetManager assetManager;
	SceneManager sceneManager;
	AudioManager audioManager;
    ImGuiLayer imGuiLayer;
    Editor editor;
    CollisionManager collisionManager;
    AIManager aiManager;

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
        &this->collisionManager,
        &this->aiManager
    };

    void Update(float deltaTime);
    void PreRender(float deltaTime);
    void Render(float deltaTime);

public:
    Application() = default;
    ~Application() = default;

    Application(const Application &other) = delete;
    Application &operator=(const Application &other) = delete;

	bool Initialize();
	void Run();
	void Shutdown();

    void Stop() { this->isRunning = false; }
};