#include "application.hpp"
#include "core/logging.hpp"
#include "input/input.hpp"
#include "core/app_context.hpp"
#include "input/action.hpp"
#include "core/debug.hpp"
#include "rendering/render_utils.hpp"

#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"

#include <iostream>
#include <fstream>
#include "yaml-cpp/yaml.h"
#include <d3d11.h>
#include "SDL3/SDL.h"

void Application::Update(float deltaTime) 
{
    Debug::BeginTimedStatistic("Update");

    if (this->editor.IsUsingEditor())
    {
        Input::SaveState();
        Input::ClearState();
        Action::Update();
    }

    if (!Debug::GetSetting("app.forceDeltaTimeZero", false)) {
        this->totalTime += deltaTime;
        this->deltaTimeElapsed += deltaTime;
        int numFixedTicks = 0;

        while (this->deltaTimeElapsed > this->fixedDeltaTime)
        {
            numFixedTicks++;
            this->deltaTimeElapsed -= this->fixedDeltaTime;
        }

        for (int i = numFixedTicks; i > 0; i--)
        {
            this->audioManager.FixedUpdate(this->fixedDeltaTime);
            this->collisionManager.FixedUpdate(this->fixedDeltaTime);
            this->aiManager.FixedUpdate(this->fixedDeltaTime);
        }

        this->sceneManager.Update(deltaTime, this->fixedDeltaTime, numFixedTicks);
    }
    else {
        this->sceneManager.Update(0.0f, 0.0f, 0);
    }

    if (this->editor.IsUsingEditor())
    {
        Input::LoadState();
        Action::Update();
    }

    this->editor.Update(this->appContext, deltaTime);

    Debug::EndTimedStatistic();
}

void Application::PreRender(float deltaTime)
{
    this->imGuiLayer.NewFrame();

    this->editor.Render(this->appContext);
}

void Application::Render(float deltaTime) 
{
    this->renderer.Render(this->sceneManager.GetCurrentScene(), this->totalTime, deltaTime);

    this->imGuiLayer.Present();
    this->renderer.Present();
}

bool Application::Initialize()
{
    LogInfo("Initializing...");
    LogIndent();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        LogError("SDL could not be initialized! SDL_Error: %s", SDL_GetError());
        return false;
    }

    if (!this->window.Initialize("Blood Princess", 1600, 900, SDL_WINDOW_RESIZABLE)) // 1400, 900
    {
        LogError("Failed to create Window!");
        return false;
    }

    if (!this->renderer.PreInitialize(this->appContext))
    {
        LogError("Failed to pre-initialize Renderer!");
        return false;
    }

    if (!this->assetManager.Initialize(&this->renderer))
    {
        LogError("Failed to create Asset Manager!");
        return false;
    }

    if (!this->renderer.Initialize(this->appContext))
    {
        LogError("Failed to create Renderer!");
        return false;
    }

    if (!this->audioManager.Initialize(&this->assetManager, this)) {
        LogError("Failed to create audio Manager!");
        return false;
    }

    if (!this->sceneManager.Initialize(this->appContext, "main_menu")) {
        LogError("Failed to create Scene Manager!");
        return false;
    }

    if (!this->collisionManager.Initialize()) {
        LogError("Failed to create collision Manager!");
        return false;
    }

#if ENABLE_IMGUI_EDITOR
    if (!this->imGuiLayer.Initialize(
        this->window.GetSdlWindow(), 
        this->renderer.GetDevice(), 
        this->renderer.GetDeviceContext())
    )
    {
        LogError("Failed to create ImGui Window");
        return false;
    }
    this->window.SetImGuiLayer(&this->imGuiLayer);

    if (!this->editor.Initialize(this->sceneManager))
    {
        LogError("Failed to create Editor");
        return false;
    }
#endif

    Input::Initialize(this->window.GetSdlWindow());
    this->isRunning = true;

    Action::Initialize();
   
    const std::string configPath = std::string(SDL_GetBasePath()) + "bindings.cfg";

    if (!Action::LoadBindings("bindings.cfg"))
    {
        Action::AddAction("MoveForward", Key::W);
        Action::AddAction("MoveBackward", Key::S);
        Action::AddAction("MoveLeft", Key::A);
        Action::AddAction("MoveRight", Key::D);

        Action::AddAction("Dash", Key::LEFT_SHIFT);
        Action::AddAction("Dash", Key::SPACE);
        Action::AddAction("Dash", GamepadButton::LEFT_SHOULDER);

        Action::AddAction("LightAttack", MouseButton::LEFT);
        Action::AddAction("LightAttack", GamepadAxis::RIGHT_TRIGGER, 0.5f);

        Action::AddAction("HeavyAttack", MouseButton::RIGHT);
        Action::AddAction("HeavyAttack", GamepadButton::RIGHT_SHOULDER);

        Action::AddAction("Parry", Key::F);
        Action::AddAction("Parry", GamepadAxis::LEFT_TRIGGER, 0.5f);

        Action::SaveBindings("bindings.cfg");
    }

    this->LoadSettings();
    LogInfo("Bindings path: %s", configPath.c_str());

    this->window.SetIcon("assets/images/test_crown.png");

    LogUnindent();
    return true;
}

void Application::Run()
{
    auto prevTime = std::chrono::high_resolution_clock::now();

    while (this->isRunning && !this->window.ShouldClose())
    {
		Input::Update(); //Has to be called before PollEvents()

        this->window.PollEvents();

        Action::Update(); //Has to be called after PollEvents()

        if (this->window.WasResized()) {
            this->renderer.Resize(this->window.GetWidth(), this->window.GetHeight());
            this->window.ClearResizeFlag();
        }

        if (Input::IsKeyPressed(Key::F11))
            this->ToggleFullscreen();

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        this->renderer.NewFrame();
        this->Update(deltaTime * this->timeScale);

        this->PreRender(deltaTime * this->timeScale);
        Debug::NewFrame();
        Debug::SetStatistic("FPS", 1.0f / deltaTime);
        Debug::SetStatistic("ms/f", 1000.0f * deltaTime);
        Debug::SetStatistic("timescale", this->timeScale);

        if (RenderView *primary = RenderUtils::GetView(this->renderer.GetViews(), ViewType::PRIMARY))
            Debug::SetStatistic("Position", primary->cameraPosition);

        this->Render(deltaTime * this->timeScale);
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

#if ENABLE_IMGUI_EDITOR
    this->imGuiLayer.Shutdown();
#endif
    this->sceneManager.Shutdown();
    this->collisionManager.Shutdown();
    this->audioManager.Shutdown();
    this->renderer.Shutdown();
    this->window.Shutdown(); 
    this->aiManager.Shutdown();
    this->assetManager.Shutdown();
    LogUnindent();
}

void Application::SetFullscreen(bool isFullscreen) {
    this->isFullscreen = isFullscreen;
    SDL_SetWindowFullscreen(this->window.GetSdlWindow(), isFullscreen);
    this->SaveSettings();
}

void Application::ToggleFullscreen() {
    this->SetFullscreen(!this->isFullscreen);
    this->SaveSettings();
}

void Application::SetMouseSensitivity(float sensitivity) {
    this->mouseSensitivity = sensitivity;
    this->SaveSettings();
}

void Application::SetInvertY(bool invertY) {
    this->invertY = invertY;
    this->SaveSettings();
}

void Application::SetMasterVolume(float volume) {
    this->masterVolume = volume;
    this->SaveSettings();
}

void Application::SetSoundVolume(float volume) {
    this->soundVolume = volume;
    this->SaveSettings();
}

void Application::SetMusicVolume(float volume) {
    this->musicVolume = volume;
    this->SaveSettings();
}

void Application::SetSkipTutorial(bool skipTutorial) {
    this->isSkipTutorial = skipTutorial;
    this->SaveSettings();
}


void Application::SetCompletedTutorial(bool compeltedTutorial) {
    if (this->hasCompeltedTutorialOnce == false && compeltedTutorial == true)
        this->isSkipTutorial = true;
    this->hasCompeltedTutorialOnce = compeltedTutorial;
    this->SaveSettings();
}

void Application::SaveSettings() {
    try {
        YAML::Node config;
        std::ifstream check("config.yaml");
        if (check.is_open()) {
            check.close();
            try {
                config = YAML::LoadFile("config.yaml");
            } catch (...) {}
        }
        config["sensitivity"] = this->mouseSensitivity;
        config["invert_y"] = this->invertY;
        config["volume_master"] = this->masterVolume;
        config["volume_sound"] = this->soundVolume;
        config["volume_music"] = this->musicVolume;
        config["fullscreen"] = this->isFullscreen;
        config["skip_tutorial"] = this->isSkipTutorial;
        config["completed_tutorial_once"] = this->hasCompeltedTutorialOnce;

        std::ofstream fout("config.yaml");
        if (fout.is_open()) {
            fout << config;
        }
    } catch (const std::exception &e) {
        LogError("Failed to save config.yaml: %s", e.what());
    }
}

void Application::LoadSettings() {
    try {
        std::ifstream check("config.yaml");
        if (!check.is_open()) {
            this->SaveSettings();
            return;
        }
        check.close();

        YAML::Node config = YAML::LoadFile("config.yaml");
        if (config["sensitivity"]) {
            this->mouseSensitivity = config["sensitivity"].as<float>();
        }
        if (config["invert_y"]) {
            this->invertY = config["invert_y"].as<bool>();
        }
        if (config["volume_master"]) {
            this->masterVolume = config["volume_master"].as<float>();
        }
        if (config["volume_sound"]) {
            this->soundVolume = config["volume_sound"].as<float>();
        }
        if (config["volume_music"]) {
            this->musicVolume = config["volume_music"].as<float>();
        }
        if (config["fullscreen"]) {
            this->isFullscreen = config["fullscreen"].as<bool>();
        }
        if (config["skip_tutorial"]) {
            this->isSkipTutorial = config["skip_tutorial"].as<bool>();
        }
        if (config["completed_tutorial_once"]) {
            this->hasCompeltedTutorialOnce = config["completed_tutorial_once"].as<bool>();
        }
    } catch (const std::exception &e) {
        LogWarn("Failed to load config.yaml: %s", e.what());
    }

    SetFullscreen(this->isFullscreen);
}
