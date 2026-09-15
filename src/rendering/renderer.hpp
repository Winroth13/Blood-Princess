#pragma once

#include "rendering/render_view.hpp"

#include <d3d11.h>
#include <Windows.h>

#include <cstdint>
#include <vector>

class Window;
class Scene;
class ImGuiWindow;

class Renderer
{
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *deviceContext = nullptr;
	IDXGISwapChain *swapChain = nullptr;
	ID3D11RenderTargetView *renderTargetView = nullptr;
	ID3D11DepthStencilView *depthStencilView = nullptr;

	D3D11_VIEWPORT viewport;
	uint32_t width = 0;
	uint32_t height = 0;
	float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f};

    std::vector<RenderView> views;

    bool CreateInterface(HWND hWnd);
	bool CreateRenderTargetView();
	bool CreateDepthStencilView();

public:
	bool Initialize(const Window *window);
	void Shutdown();

    void NewFrame();
	void Render(Scene *scene);
    void Present();

    bool Resize(uint32_t width, uint32_t height);

    void AddView(RenderView &view);

    float GetAspectRatio() const { return this->width / (float)this->height; }

	ID3D11Device *GetDevice() const { return this->device; }
	ID3D11DeviceContext *GetDeviceContext() const { return this->deviceContext; }
};