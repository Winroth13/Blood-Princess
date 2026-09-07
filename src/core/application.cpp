#include "application.hpp"
#include "core/logging.hpp"
#include "input.hpp"
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
        LogError("Failed to create window!");
        return false;
    }

    if (!this->renderer.Initialize(this->window))
    {
        LogError("Failed to create renderer!");
        return false;
    }

    Input::Initialize();
    this->isRunning = true;

    LogUnindent();
    return true;
}

void Application::Run()
{
    auto prevTime = std::chrono::high_resolution_clock::now();

    while (this->isRunning && !this->window.ShouldClose())
    {
        Input::Update(this->window.GetSdlWindow());

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

        this->sceneManager.Update(deltaTime);
        this->renderer.Render();
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

    this->renderer.Shutdown();
    this->window.Shutdown();

    LogUnindent();
}