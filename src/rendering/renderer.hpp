#pragma once

#include "core/window.hpp"

#include <d3d11.h>
#include <Windows.h>

class Renderer
{
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *deviceContext = nullptr;
	IDXGISwapChain *swapChain = nullptr;
	ID3D11RenderTargetView *renderTargetView = nullptr;

	D3D11_VIEWPORT viewport;
	uint32_t width = 0;
	uint32_t height = 0;
	float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f};

    bool CreateInterface(HWND hWnd);
	bool CreateRenderTargetView();

public:
	bool Initialize(const Window &window);
	void Shutdown();
	void Render() {}
};