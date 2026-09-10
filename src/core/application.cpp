#include "application.hpp"
#include "core/logging.hpp"
#include "input.hpp"
#include "core/app_context.hpp"

#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"

#include <iostream>
#include <d3d11.h>
#include "SDL3/SDL.h"

void Application::Update(float deltaTime) {
    this->sceneManager.Update(deltaTime);
}

void Application::Render() {
    this->renderer.Render();
    this->imGuiWindow.Render();

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
    if (!this->audioManager.Initialize(&this->assetManager)) {
        LogError("Failed to create audio Manager!");
        return false;
    }

    if (!this->sceneManager.Initialize(this->appContext, "demo_0")) {
        LogError("Failed to create Scene Manager!");
        return false;
    }

    if (!this->imGuiWindow.Initialize(
        this->window.GetSdlWindow(), 
        this->renderer.GetDevice(), 
        this->renderer.GetDeviceContext())
    )
    {
        LogError("Failed to create ImGui Window");
        return false;
    }
    this->window.SetImGuiWindow(&this->imGuiWindow);

    Input::Initialize();
    this->isRunning = true;

    Input::SetMouseMode(MouseMode::VISIBLE, this->window.GetSdlWindow());
    auto texture = assetManager.Get<Texture2D>("assets/princess.png");
    auto texture2 = assetManager.Get<Texture2D>("assets/princess2.png");

    //auto model = assetManager.Get<Model>("assets/princess.glb");
    auto shader = assetManager.Get<VertexShader>("src/shaders/ForwardVS.cso");
    auto pixelShader = assetManager.Get<PixelShader>("src/shaders/ForwardPS.cso");

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

        if (this->window.WasResized()) {
            this->renderer.Resize(this->window.GetWidth(), this->window.GetHeight());
            this->window.ClearResizeFlag();
        }

        if (Input::IsKeyPressed(SDL_SCANCODE_F11))
        {
            static bool isFullscreen = false;
            isFullscreen = !isFullscreen;
            SDL_SetWindowFullscreen(this->window.GetSdlWindow(), isFullscreen);
        }

        if (Input::IsKeyReleased(SDL_SCANCODE_F))
        {
            LogInfo("F key is released");
        }

        if (Input::IsMouseButtonReleased(MouseButton::LEFT)) {
            LogInfo("LMB Released");
        }
        const DirectX::XMFLOAT2 wheel = Input::GetMouseWheel();
        if (wheel.x != 0.0f || wheel.y != 0.0f)
        {
            LogInfo("Wheel: %.1f, %.1f", wheel.x, wheel.y);
        }
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float deltaTime = elapsed.count();
        prevTime = currentTime;

        this->renderer.NewFrame();

        this->Update(deltaTime);

        this->Render();
    }
}

void Application::Shutdown()
{
    LogBlank();
    LogInfo("Shutting down...");
    LogIndent();

    this->imGuiWindow.Shutdown();
    this->sceneManager.Shutdown();
    this->assetManager.Shutdown();
    this->renderer.Shutdown();
    this->window.Shutdown(); 

    LogUnindent();
}