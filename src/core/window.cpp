#include "core/window.hpp"
#include "core/logging.hpp"
#include "input/input.hpp"
#include "core/application.hpp"

#include "stb_image/stb_image.h"

bool Window::Initialize(
    const std::string& title,
    uint32_t width,
    uint32_t height,
    SDL_WindowFlags flags
)
{
    LogInfo("Creating window...");
    LogIndent();

    this->sdlWindow = SDL_CreateWindow(title.c_str(), width, height, flags);

    if (!this->sdlWindow)
    {
        LogError("SDL failed to create window! SDL_ERROR: %s", SDL_GetError());
        return false;
    }

    this->width = width;
    this->height = height;
    this->shouldClose = false;

    LogUnindent();
    return true;
}

void Window::Shutdown()
{
    LogInfo("Shutting down window...");
    LogIndent();

    SDL_free(this->iconSurface);
    delete this->iconData;

    LogInfo("Destroying SDL Window");
    SDL_DestroyWindow(this->sdlWindow);

    LogUnindent();
}

void Window::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
#if ENABLE_IMGUI_EDITOR
        this->imGuiLayer->PollEvent(&event);
#endif

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            this->shouldClose = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        {
            this->wasResized = true;

            int width, height;
            SDL_GetWindowSizeInPixels(this->sdlWindow, &width, &height);
            this->width = width;
            this->height = height;
        } break;
        }

        Input::HandleEvent(event);
    }
}

void Window::SetIcon(const std::string &path) {
    int width, height, channels;
    this->iconData = (char *)stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!this->iconData) {
        LogWarn("Failed to load icon image");
        return;
    }
    
    this->iconSurface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_ABGR8888, this->iconData, width * 4);
    if (!this->iconSurface) {
        LogWarn("Failed to create icon surface");
        return;
    }

    if (!SDL_SetWindowIcon(this->sdlWindow, this->iconSurface)) {
        LogWarn("Failed to set window icon");
        return;
    }
}

HWND Window::GetHWNDPointer() const
{
    return (HWND)SDL_GetPointerProperty(
        SDL_GetWindowProperties(sdlWindow),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        NULL
    );
}