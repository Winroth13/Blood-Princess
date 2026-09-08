#include "application.hpp"
#include "core/logging.hpp"
#include "input.hpp"
#include "core/app_context.hpp"

#include <iostream>
#include <d3d11.h>
#include "SDL3/SDL.h"

bool Application::Initialize()
{
    LogInfo("Initializing...");
    LogIndent();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogError("SDL could not be initialized! SDL_Error: %s", SDL_GetError());
        return false;
    }

    if (!this->window.Initialize("Blood Princess", 800, 600, SDL_WINDOW_RESIZABLE))
    {
        LogError("Failed to create Window!");
        return false;
    }

    if (!this->renderer.Initialize(&this->window))
    {
        LogError("Failed to create Renderer!");
        return false;
    }

    if (!this->assetManager.Initialize(&this->renderer))
    {
        LogError("Failed to create Asset Manager!");
        return false;
    }

    if (!this->sceneManager.Initialize(this->appContext, "demo_0")) {
        LogError("Failed to create Scene Manager!");
        return false;
    }

    Input::Initialize();
    this->isRunning = true;
    Input::SetMouseMode(MouseMode::Visible, this->window.GetSdlWindow());
    auto texture = assetManager.Get<Texture2D>("princess.png");
    auto texture2 = assetManager.Get<Texture2D>("princess2.png");

    auto model = assetManager.Get<Model>("princess.glb");

    LogUnindent();
    return true;
}

void Application::Run()
{
    auto prevTime = std::chrono::high_resolution_clock::now();

    while (this->isRunning && !this->window.ShouldClose())
    {
		Input::Update(this->window.GetSdlWindow()); //Has to be called before PollEvents()

        this->window.PollEvents();
        if (Input::IsKeyPressed(SDL_SCANCODE_F))
        {
			LogInfo("F key is pressed");
        }

        if (Input::IsKeyReleased(SDL_SCANCODE_F))
        {
            LogInfo("F key is released");
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        this->sceneManager.Update(this->appContext, deltaTime);
        this->renderer.Render();
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

    this->sceneManager.Shutdown();
    this->assetManager.Shutdown();
    this->renderer.Shutdown();
    this->window.Shutdown();

    LogUnindent();
}