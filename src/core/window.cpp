#include "core/window.hpp"
#include "core/logging.hpp"
#include "input.hpp"

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
<<<<<<< HEAD
        case SDL_EVENT_QUIT:
            this->shouldClose = true;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
			Input::OnMouseWheelEvent(event.wheel.x, event.wheel.y);
			break;
=======
            case SDL_EVENT_QUIT:
                this->shouldClose = true;
                break;
            case SDL_EVENT_WINDOW_RESIZED: {
                this->wasResized = true;

                int width, height;
                SDL_GetWindowSizeInPixels(this->sdlWindow, &width, &height);
                this->width = width;
                this->height = height;
            } break;
>>>>>>> 895ef2cdbc86f2a6157e7d06057ef3b635f71a30
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
