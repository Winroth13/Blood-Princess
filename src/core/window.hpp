#pragma once
#include "sdl3/include/SDL3/SDL.h"

#include "editor/imgui_layer.hpp"

#include <string>
#include <Windows.h>

class Window
{
	SDL_Window *sdlWindow = nullptr;
	ImGuiLayer *imGuiLayer = nullptr;

	bool shouldClose = true;
    bool wasResized = false;

	uint32_t width = 0;
	uint32_t height = 0;

    char *iconData = nullptr;
    SDL_Surface *iconSurface = nullptr;

public:
    Window() = default;
	~Window() = default;

    Window(const Window &other) = default;
    Window &operator=(const Window &other) = default;

	bool Initialize(const std::string &title, uint32_t width, uint32_t height, SDL_WindowFlags flags = 0);
	void SetImGuiLayer(ImGuiLayer *imGuiLayer) { this->imGuiLayer = imGuiLayer; }
	void Shutdown();

	void PollEvents();

    void SetIcon(const std::string &path);

    bool WasResized() const { return this->wasResized; }
    void ClearResizeFlag() { this->wasResized = false; }

	bool ShouldClose() const { return this->shouldClose; }
	HWND GetHWNDPointer() const;
	SDL_Window* GetSdlWindow() const { return this->sdlWindow; }

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }
};