#pragma once
#include "SDL3/SDL.h"

struct SDL_Window;
struct ID3D11Device;
struct ID3D11DeviceContext;

class ImGuiWindow
{
public:
	ImGuiWindow() = default;
	~ImGuiWindow() = default;

	bool Initialize(
		SDL_Window *sdlWindow,
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext
	);

	void Update();

	void PollEvent(SDL_Event *event);
};