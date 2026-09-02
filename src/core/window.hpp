#pragma once
#include "sdl3/include/SDL3/SDL.h"

#include <string>
#include <Windows.h>

class Window
{
	SDL_Window* sdlWindow = nullptr;
	bool shouldClose = true;

	uint32_t width = 0;
	uint32_t height = 0;

public:
    Window() = default;
	~Window() = default;

    Window(const Window &other) = default;
    Window &operator=(const Window &other) = default;

	bool Initialize(const std::string &title, uint32_t width, uint32_t height, SDL_WindowFlags flags = 0);
	void Shutdown();

	void PollEvents();

	bool ShouldClose() const { return this->shouldClose; }
	HWND GetHWNDPointer() const;

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }
};