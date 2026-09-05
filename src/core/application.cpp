#include "application.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"

#include <iostream>
#include <d3d11.h>

bool Application::Initialize()
{
    LogInfo("Initializing...");
    LogIndent();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogError("SDL could not be initialized! SDL_Error: %s", SDL_GetError());
        return false;
    }

    this->appContext = std::make_unique<AppContext>();

    if (!this->window.Initialize("Blood Princess", 800, 600, SDL_WINDOW_RESIZABLE))
    {
        LogError("Failed to create Window!");
        return false;
    }
    this->appContext->window = &this->window;

    if (!this->renderer.Initialize(this->window))
    {
        LogError("Failed to create Renderer!");
        return false;
    }
    this->appContext->renderer = &this->renderer;

    if (!this->assetManager.Initialize(this->appContext.get()))
    {
        LogError("Failed to create Asset Manager!");
        return false;
    }

    this->isRunning = true;

    auto texture = assetManager.Get<Texture2D>("princess.png");
    auto texture2 = assetManager.Get<Texture2D>("princess2.png");

    LogUnindent();
    return true;
}

void Application::Run()
{
    auto prevTime = std::chrono::high_resolution_clock::now();

    while (this->isRunning && !this->window.ShouldClose())
    {
        this->window.PollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        this->sceneManager.Update(deltaTime);
        this->renderer.Render();
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

    this->assetManager.Shutdown();
    this->renderer.Shutdown();
    this->window.Shutdown();

    LogUnindent();
}