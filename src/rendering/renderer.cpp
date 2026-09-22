#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"
#include "core/window.hpp"
#include "core/logging.hpp"
#include "scene/scene.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/model.hpp"
#include "editor/imgui_layer.hpp"

#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>
#include <vector>

using namespace DirectX;

bool Renderer::CreateInterface(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};

    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // _SRGB
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    UINT flags = 0;
#if _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG; 
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0
    };

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &this->swapChain,
        &this->device,
        nullptr,
        &this->deviceContext
    );

    if (FAILED(result)) 
    {
        LogError("Failed to create the Device or Swap Chain");
        return false;
    }

    RenderUtils::SetDebugName(this->device, DebugCategory::CORE, "Device");
    RenderUtils::SetDebugName(this->deviceContext, DebugCategory::CORE, "Device Context");
    RenderUtils::SetDebugName(this->swapChain, DebugCategory::CORE, "Swap Chain");

    LogInfo("Device and Swap Chain created");

    return true;
}

bool Renderer::PreInitialize(const AppContext &appContext)
{
    LogInfo("Pre Initializing Renderer...");
    LogIndent();

    this->width = appContext.window->GetWidth();
    this->height = appContext.window->GetHeight();

    if (!this->CreateInterface(appContext.window->GetHWNDPointer()))
    {
        LogError("Failed to create Interface");
        return false;
    }

    InitializeAnnotator(this->deviceContext);

    if (!this->backBuffer.Create(this->device, this->swapChain))
    {
        LogError("Failed to create Back Buffer");
        return false;
    }

    this->viewport.TopLeftX = 0.0f;
    this->viewport.TopLeftY = 0.0f;
    this->viewport.Width = static_cast<float>(this->width);
    this->viewport.Height = static_cast<float>(this->height);
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;

    LogUnindent();

    return true;
}

bool Renderer::Initialize(const AppContext &appContext)
{
    LogInfo("Creating Renderer...");
    LogIndent();

    if (!this->sharedResources.Initialize(this->device))
    {
        LogError("Failed to create Shared Resources");
        return false;
    }

    if (!this->deferredPass.Initialize(this->device, appContext.assetManager, this->width, this->height))
    {
        LogError("Failed to create Deferred Pass");
        return false;
    }

    if (!this->particlePass.Initialize(this->device))
    {
        LogError("Failed to create Particle Pass");
        return false;
    }

    LogUnindent();

	return true;
}

void Renderer::Shutdown()
{
    LogInfo("Shutting down Renderer...");
    LogIndent();

    ShutdownAnnotator();

    this->sharedResources.Shutdown();

    this->deferredPass.Shutdown();
    this->particlePass.Shutdown();

    LogInfo("Releasing Swap Chain");
    SafeRelease(this->swapChain);

    LogInfo("Releasing Device Context");
    SafeRelease(this->deviceContext);

    LogInfo("Releasing Device");
    SafeRelease(this->device);

    LogUnindent();
}

void Renderer::NewFrame()
{
    this->views.clear();
}

void Renderer::Render(Scene *scene)
{
    if (!scene) {
        LogError("Scene was nullptr\n");
        return;
    }

    this->deviceContext->ClearRenderTargetView(this->backBuffer.GetView(), this->clearColor);

    RenderView *primary = RenderUtils::GetView(this->views, ViewType::PRIMARY);
    if (!primary)
        LogWarn("No primary view was found");

    scene->GatherRenderCommands(this->views);

    BeginAnnotation("Deferred Pass");

    this->deferredPass.Render(
        this->device, 
        this->deviceContext, 
        this->sharedResources,
        this->backBuffer,
        this->views
    );

    EndAnnotation();

    BeginAnnotation("Particle Pass");

    this->particlePass.Render(
        this->device,
        this->deviceContext,
        this->sharedResources,
        this->backBuffer.GetView(),
        this->views
    );

    EndAnnotation();

    ID3D11RenderTargetView *view = this->backBuffer.GetView();
    this->deviceContext->OMSetRenderTargets(1, &view, nullptr);
}

void Renderer::Present()
{
    this->swapChain->Present(0, 0);
}

bool Renderer::Resize(uint32_t width, uint32_t height)
{
    if (width <= 0 || height <= 0)
        return false;

    LogBlank();
    LogInfo("Resizing window...");
    LogIndent();

    this->width = width;
    this->height = height;

    this->viewport.Width = static_cast<float>(this->width);
    this->viewport.Height = static_cast<float>(this->height);

    this->backBuffer.Release();

    this->deviceContext->ClearState();
    this->deviceContext->Flush();

    HRESULT result = this->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(result))
    {
        LogError("Failed to resize Swap Chain Buffers");
        return false;
    }

    if (!this->backBuffer.Create(this->device, this->swapChain))
    {
        LogError("Failed to create Back Buffer");
        return false;
    }

    if (!this->deferredPass.Resize(this->device, this->width, this->height))
    {
        LogError("Failed to resize Deferred Pass");
        return false;
    }

    // this->SetViewport(width, height);
    // this->frameGraph.OnResize(width, height);

    LogUnindent();
    return true;
}

void Renderer::AddView(RenderView &view)
{
    BoundingFrustum::CreateFromMatrix(view.frustum, XMLoadFloat4x4(&view.projectionMatrix));
    view.frustum.Transform(view.frustum, XMMatrixInverse(nullptr, XMLoadFloat4x4(&view.viewMatrix)));

    if (view.type == ViewType::PRIMARY)
    {
        for (RenderView &other : this->views)
        {
            if (other.type == ViewType::PRIMARY)
            {
                other = view;
                return;
            }
        }
    }

    this->views.push_back(view);
}