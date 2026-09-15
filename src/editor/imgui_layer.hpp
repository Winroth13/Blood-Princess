#pragma once
#include "SDL3/SDL.h"

struct SDL_Window;
struct ID3D11Device;
struct ID3D11DeviceContext;

class ImGuiLayer
{
public:
	ImGuiLayer() = default;
	~ImGuiLayer() = default;

	bool Initialize(
		SDL_Window *sdlWindow,
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext
	);

	void NewFrame();
	void Present();

    void Shutdown();

	void PollEvent(SDL_Event *event);
};