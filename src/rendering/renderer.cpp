#include "rendering/renderer.hpp"
#include "core/window.hpp"
#include "core/logging.hpp"

#define SafeRelease(obj) do { if (obj) (obj)->Release(); (obj) = nullptr; } while (0)

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

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
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

    LogInfo("Device and Swap Chain created");

    return true;
}

bool Renderer::CreateRenderTargetView()
{
    ID3D11Texture2D *backbuffer{};
    HRESULT result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

    if (FAILED(result))
    {
        LogError("Failed to create the backbuffer");
        return false;
    }

    LogInfo("Backbuffer created");

    result = this->device->CreateRenderTargetView(backbuffer, nullptr, &this->renderTargetView);
    backbuffer->Release();

    if (FAILED(result)) 
    {
        LogError("Failed to create the Render Target View");
        return false;
    }

    LogInfo("Render Target View created");
    return true;
}

bool Renderer::Initialize(const Window &window)
{
    LogInfo("Creating Renderer...");
    LogIndent();

    this->width = window.GetWidth();
    this->height = window.GetHeight();

    if (!this->CreateInterface(window.GetHWNDPointer()))
        return false;

    if (!this->CreateRenderTargetView())
        return false;

    this->deviceContext->ClearRenderTargetView(this->renderTargetView, this->clearColor);
    this->swapChain->Present(0, 0);

    LogUnindent();

	return true;
}

void Renderer::Shutdown()
{
    LogInfo("Shutting down Renderer...");
    LogIndent();

    LogInfo("Releasing Render Target View");
    SafeRelease(this->renderTargetView);

    LogInfo("Releasing Swap Chain");
    SafeRelease(this->swapChain);

    LogInfo("Releasing Device Context");
    SafeRelease(this->deviceContext);

    LogInfo("Releasing Device");
    SafeRelease(this->device);

    LogUnindent();
}
