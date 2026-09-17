#include "application.hpp"
#include "core/logging.hpp"
#include "input/input.hpp"
#include "core/app_context.hpp"
#include "input/action.hpp"
#include "input/action_names.hpp"

#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"

#include <iostream>
#include <d3d11.h>
#include "SDL3/SDL.h"

void Application::Update(float deltaTime) {
    this->sceneManager.Update(deltaTime);
	this->audioManager.Update(deltaTime);
}

void Application::Render() {
    this->renderer.Render(this->sceneManager.GetCurrentScene());
    this->imGuiLayer.Present();
    this->renderer.Present();
}

bool Application::Initialize()
{
    LogInfo("Initializing...");
    LogIndent();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogError("SDL could not be initialized! SDL_Error: %s", SDL_GetError());
        return false;
    }

    if (!this->window.Initialize("Blood Princess", 1400, 900, SDL_WINDOW_RESIZABLE))
    {
        LogError("Failed to create Window!");
        return false;
    }

    if (!this->renderer.Initialize(this->appContext))
    {
        LogError("Failed to create Renderer!");
        return false;
    }

    if (!this->assetManager.Initialize(&this->renderer))
    {
        LogError("Failed to create Asset Manager!");
        return false;
    }

    if (!this->audioManager.Initialize(&this->assetManager)) {
        LogError("Failed to create audio Manager!");
        return false;
    }

    if (!this->sceneManager.Initialize(this->appContext, "demo_0")) {
        LogError("Failed to create Scene Manager!");
        return false;
    }

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

    Input::Initialize();
    this->isRunning = true;

    Action::Initialize();

    if (!Action::LoadBindings("bindings.txt"))
    {
        Action::AddAction(Actions::MOVE_FORWARD, Key::W);
        Action::AddAction(Actions::MOVE_BACKWARD, Key::S);
        Action::AddAction(Actions::MOVE_LEFT, Key::A);
        Action::AddAction(Actions::MOVE_RIGHT, Key::D);

        Action::AddAction(Actions::DODGE, Key::LEFT_SHIFT);

        Action::AddAction(Actions::LIGHT_ATTACK, MouseButton::LEFT);
        Action::AddAction(Actions::HEAVY_ATTACK, MouseButton::RIGHT);

        Action::SaveBindings("bindings.txt");

    }


    Input::SetMouseMode(MouseMode::VISIBLE, this->window.GetSdlWindow());

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
        {
            static bool isFullscreen = false;
            isFullscreen = !isFullscreen;
            SDL_SetWindowFullscreen(this->window.GetSdlWindow(), isFullscreen);
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        this->renderer.NewFrame();
        this->imGuiLayer.NewFrame();

        this->Update(deltaTime);

        this->editor.ImGuiRender(this->appContext);

        this->Render();
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

    this->imGuiLayer.Shutdown();
    this->sceneManager.Shutdown();
    this->assetManager.Shutdown();
    this->audioManager.Shutdown();
    this->renderer.Shutdown();
    this->window.Shutdown(); 

    LogUnindent();
}