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
        Action::AddAction("Dash", GamepadButton::EAST);

        Action::AddAction("LightAttack", MouseButton::LEFT);
        Action::AddAction("LightAttack", GamepadButton::LEFT_SHOULDER);

        Action::AddAction("HeavyAttack", MouseButton::RIGHT);
        Action::AddAction("HeavyAttack", GamepadButton::RIGHT_SHOULDER);

        Action::AddAction("Parry", Key::F);
        Action::AddAction("Parry", GamepadButton::WEST);

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
}

void Application::ToggleFullscreen() {
    this->SetFullscreen(!this->isFullscreen);
}
void Application::SetMouseSensitivity(float sensitivity) {
    this->mouseSensitivity = sensitivity;
    this->SaveSettings();
}

void Application::SetInvertY(bool invertY) {
    this->invertY = invertY;
    this->SaveSettings();
}

void Application::SaveSettings() {
    std::ofstream file("settings.cfg");
    if (!file.is_open())
        return;
    file << "mouse_sensitivity " << this->mouseSensitivity << "\n";
    file << "invert_y " << (this->invertY ? 1 : 0) << "\n";
}

void Application::LoadSettings() {
    std::ifstream file("settings.cfg");
    if (!file.is_open())
        return;
    std::string key;
    while (file >> key) {
        if (key == "mouse_sensitivity") {
            file >> this->mouseSensitivity;
        } else if (key == "invert_y") {
            int inv = 0;
            file >> inv;
            this->invertY = (inv != 0);
        }
    }
}
