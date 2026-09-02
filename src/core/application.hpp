#pragma once

#include "core/window.hpp"
#include "rendering/renderer.hpp"
#include "resources/asset_manager.hpp"
#include "scene/scene_manager.hpp"
#include "audio/audio_manager.hpp"

#include <chrono>

class Application 
{
	Window window;
	Renderer renderer;
	AssetManager assetManager;
	SceneManager sceneManager;
	AudioManager audioManager;

	bool isRunning = false;

public:
    Application() = default;
    ~Application() = default;

    Application(const Application &other) = delete;
    Application &operator=(const Application &other) = delete;

	bool Initialize();
	void Run();
	void Shutdown();
};