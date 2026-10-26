#pragma once

#include "core/window.hpp"
#include "core/game_state.hpp"
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

    float timeScale = 1.0f;

	Window window;
	Renderer renderer;
	AssetManager assetManager;
	SceneManager sceneManager;
	AudioManager audioManager;
    ImGuiLayer imGuiLayer;
    Editor editor;
    CollisionManager collisionManager;
    AIManager aiManager;
    GameState gameState;

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

    bool isFullscreen = false;
    float mouseSensitivity = 0.1f;
    bool invertY = false;

    float masterVolume = 1.0f;
    float soundVolume = 1.0f;
    float musicVolume = 1.0f;

    bool isSkipTutorial = false;
    bool hasCompeltedTutorialOnce = false;

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

    bool IsFullscreen() const { return this->isFullscreen; }
    void SetFullscreen(bool isFullscreen);
    void ToggleFullscreen();

    float GetMouseSensitivity() const { return this->mouseSensitivity; }
    void SetMouseSensitivity(float sensitivity);

    bool GetInvertY() const { return this->invertY; }
    void SetInvertY(bool invertY);

    float GetMasterVolume() const { return this->masterVolume; }
    void SetMasterVolume(float volume);

    float GetSoundVolume() const { return this->soundVolume; }
    void SetSoundVolume(float volume);

    float GetMusicVolume() const { return this->musicVolume; }
    void SetMusicVolume(float volume);

    bool IsSkipTutorial() const { return this->isSkipTutorial; }
    void SetSkipTutorial(bool skipTutorial);

    bool HasCompletedTutorialOnce() const { return this->hasCompeltedTutorialOnce; }
    void SetCompletedTutorial(bool compeltedTutorial);
    
    void SaveSettings();
    void LoadSettings();

    float GetTimeScale() const { return this->timeScale; }
    void SetTimeScale(float timescale) { this->timeScale = timescale; }

    GameState *GetGameState() { return &this->gameState; }
};
