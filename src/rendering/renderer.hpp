#pragma once

#include "rendering/render_view.hpp"
#include "resources/backbuffer.hpp"
#include "passes/deferred_pass.hpp"
#include "passes/particle_pass.hpp"
#include "passes/shadow_pass.hpp"
#include "rendering/shared_resources.hpp"
#include "core/app_context.hpp"
#include "rendering/passes/output_pass.hpp"
#include "rendering/passes/line_pass.hpp"
#include "rendering/passes/paint_pass.hpp"
#include "rendering/passes/ui_pass.hpp"

#include <d3d11.h>
#include <Windows.h>

#include <cstdint>
#include <vector>

class Scene;

class Renderer
{
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *deviceContext = nullptr;
	IDXGISwapChain *swapChain = nullptr;

	SharedResources sharedResources;

	BackBuffer backBuffer;

	DeferredPass deferredPass;
	ParticlePass particlePass;
	ShadowPass shadowPass;
    OutputPass outputPass;
	LinePass linePass;
	PaintPass paintPass;
    UIPass uiPass;

	D3D11_VIEWPORT viewport;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t shadowSize = 256;
	uint32_t shadowSizeDirectional = 4096;
	float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f};

    std::vector<RenderView> views;

    bool CreateInterface(HWND hWnd);

public:
	bool PreInitialize(const AppContext &appContext);
	bool Initialize(const AppContext &appContext);
	void Shutdown();

    void NewFrame();
	void Render(Scene *scene, float totalTime, float deltaTime);
    void Present();

    bool Resize(uint32_t width, uint32_t height);

    void AddView(RenderView &view);
	std::vector<RenderView> &GetViews() { return this->views; }

    float GetAspectRatio() const { return this->width / (float)this->height; }

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }

	ID3D11Device *GetDevice() const { return this->device; }
	ID3D11DeviceContext *GetDeviceContext() const { return this->deviceContext; }
};