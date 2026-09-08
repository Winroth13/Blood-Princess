#include "core/window.hpp"
#include "core/logging.hpp"

bool Window::Initialize(
    const std::string &title, 
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

    LogInfo("Destroying SDL Window");
    SDL_DestroyWindow(this->sdlWindow);

    LogUnindent();
}

void Window::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        imGuiWindow->PollEvent(&event);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            this->shouldClose = true;
            break;
        }
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
