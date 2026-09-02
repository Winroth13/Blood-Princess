#include "application.hpp"
#include <iostream>

#include "sdl3/include/SDL3/SDL.h"

bool Application::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not be initialized!" << std::endl
            << "SDL_Error: " << SDL_GetError() << std::endl;

        return false;
    }

    isRunning = true;
    return true;
}

void Application::Run()
{
    auto prevTime = std::chrono::high_resolution_clock::now();

    SDL_Window* sdlWindow = SDL_CreateWindow("Engine", 800, 600, 0);

    while (isRunning)
    {
        window.PollEvents();

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        sceneManager.Update(deltaTime);
        renderer.Render();
    }
}

void Application::Shutdown()
{

}