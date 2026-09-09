#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"
#include "core/window.hpp"
#include "core/logging.hpp"
#include "scene/scene.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/model.hpp"
#include "editor/imgui_window.hpp"

#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>
#include <vector>

using namespace DirectX;

// Camera & projection parameters
static const float FOV = XMConvertToRadians(45.0f);
static const float ASPECT_RATIO = 1024.0f / 576.0f;
static const float NEAR_PLANE = 0.1f;
static const float FAR_PLANE = 100.0f;

static const XMVECTOR EYE = XMVectorSet(0.0f, 0.0f, 3.0f, 1.0f);
static const XMVECTOR AT = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
static const XMVECTOR UP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

static const XMMATRIX VIEW = XMMatrixLookAtLH(EYE, AT, UP);
static const XMMATRIX PROJECTION = XMMatrixPerspectiveFovLH(FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
static const XMMATRIX VIEW_PROJ = VIEW * PROJECTION;

struct MatrixPair
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 viewProj;
};

namespace
{
	ID3D11VertexShader* vShader = nullptr;
	ID3D11PixelShader* pShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11SamplerState* samplerState = nullptr;
}
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

bool Renderer::CreateDepthStencilView()
{
    D3D11_TEXTURE2D_DESC dsDesc{};
    dsDesc.Width = this->width;
    dsDesc.Height = this->height;
    dsDesc.MipLevels = 1;
    dsDesc.ArraySize = 1;
    dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsDesc.SampleDesc.Count = 1;
    dsDesc.SampleDesc.Quality = 0;
    dsDesc.Usage = D3D11_USAGE_DEFAULT;
    dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D *dsTexture = nullptr;
    HRESULT hr = this->device->CreateTexture2D(&dsDesc, nullptr, &dsTexture);
    if (FAILED(hr))
    {
        LogError("Failed to create the Depth Stencil Texture");
        return false;
    }

    LogInfo("Depth Stencil Texture created");

    hr = this->device->CreateDepthStencilView(dsTexture, nullptr, &this->depthStencilView);
    dsTexture->Release();

    if (FAILED(hr))
    {
        LogError("Failed to create the Depth Stencil View");
        return false;
    }

    LogInfo("Depth Stencil View created");
    return true;
}

bool Renderer::Initialize(const Window *window)
{
    LogInfo("Creating Renderer...");
    LogIndent();

    this->width = window->GetWidth();
    this->height = window->GetHeight();

    if (!this->CreateInterface(window->GetHWNDPointer()))
        return false;

    if (!this->CreateRenderTargetView())
        return false;

    if (!this->CreateDepthStencilView())
        return false;

    this->viewport.TopLeftX = 0.0f;
    this->viewport.TopLeftY = 0.0f;
    this->viewport.Width = static_cast<float>(this->width);
    this->viewport.Height = static_cast<float>(this->height);
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;

    // Load compiled shaders from .cso files (matching lab implementation)
    auto loadShader = [](const std::string& filename) -> std::vector<char>
    {
        std::ifstream reader(filename, std::ios::binary | std::ios::ate);
        if (!reader.is_open())
            return {};
        size_t size = reader.tellg();
        std::vector<char> buffer(size);
        reader.seekg(0);
        reader.read(buffer.data(), size);
        return buffer;
    };

    auto vertexShaderData = loadShader("src/shaders/ForwardVS.cso");
    if (vertexShaderData.empty())
    {
        LogError("Failed to load src/shaders/ForwardVS.cso");
        return false;
    }
    this->device->CreateVertexShader(vertexShaderData.data(), vertexShaderData.size(), nullptr, &vShader);
    LogInfo("Vertex Shader created");

    auto pixelShaderData = loadShader("src/shaders/ForwardPS.cso");
    if (pixelShaderData.empty())
    {
        LogError("Failed to load src/shaders/ForwardPS.cso");
        return false;
    }
    this->device->CreatePixelShader(pixelShaderData.data(), pixelShaderData.size(), nullptr, &pShader);
    LogInfo("Pixel Shader created");

    // Input layout
    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    this->device->CreateInputLayout(inputDesc, 3, vertexShaderData.data(), vertexShaderData.size(), &inputLayout);
    LogInfo("Input Layout created");

    // Matrix constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(MatrixPair);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    this->device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    LogInfo("Constant Buffer created");

    // Sampler state
    D3D11_SAMPLER_DESC sdesc = {};
    sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sdesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sdesc.MaxLOD = D3D11_FLOAT32_MAX;
    this->device->CreateSamplerState(&sdesc, &samplerState);
    LogInfo("Sampler State created");

    this->deviceContext->ClearRenderTargetView(this->renderTargetView, this->clearColor);
    this->swapChain->Present(0, 0);

    LogUnindent();

	return true;
}

void Renderer::Shutdown()
{
    LogInfo("Shutting down Renderer...");
    LogIndent();

    LogInfo("Releasing Sampler State");
    SafeRelease(samplerState);

    LogInfo("Releasing Constant Buffer");
    SafeRelease(constantBuffer);

    LogInfo("Releasing Input Layout");
    SafeRelease(inputLayout);

    LogInfo("Releasing Pixel Shader");
    SafeRelease(pShader);

    LogInfo("Releasing Vertex Shader");
    SafeRelease(vShader);

    LogInfo("Releasing Depth Stencil View");
    SafeRelease(this->depthStencilView);

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

void Renderer::NewFrame()
{
    this->views.clear();
}

void Renderer::Render()
{
    if (this->views.empty()) {
        LogError("Oh no!!!!!!!!!");
    }

    this->deviceContext->ClearRenderTargetView(this->renderTargetView, this->clearColor);
    this->deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    this->deviceContext->OMSetRenderTargets(1, &this->renderTargetView, nullptr);

    //if (!scene)
    //{
    //    this->swapChain->Present(0, 0);
    //    return;
    //}

    //auto mesh = scene->GetMesh();
    //if (!mesh || !mesh->GetVertexBuffer() || !mesh->GetIndexBuffer())
    //{
    //    this->swapChain->Present(0, 0);
    //    return;
    //}

    //// Update Matrix constant buffer with the scene's transform
    //XMMATRIX worldT = XMMatrixTranspose(scene->GetWorldMatrix());
    //XMMATRIX viewProjT = XMMatrixTranspose(VIEW_PROJ);

    //D3D11_MAPPED_SUBRESOURCE mapped;
    //if (SUCCEEDED(this->deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    //{
    //    MatrixPair data;
    //    XMStoreFloat4x4(&data.world, worldT);
    //    XMStoreFloat4x4(&data.viewProj, viewProjT);
    //    memcpy(mapped.pData, &data, sizeof(MatrixPair));
    //    this->deviceContext->Unmap(constantBuffer, 0);
    //}

    //this->deviceContext->RSSetViewports(1, &this->viewport);
    //this->deviceContext->OMSetRenderTargets(1, &this->renderTargetView, this->depthStencilView);

    //ID3D11Buffer *vBuffer = mesh->GetVertexBuffer();
    //UINT stride = sizeof(Vertex);
    //UINT offset = 0;
    //this->deviceContext->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
    //this->deviceContext->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
    //this->deviceContext->IASetInputLayout(inputLayout);
    //this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //this->deviceContext->VSSetShader(vShader, nullptr, 0);
    //this->deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    //this->deviceContext->PSSetShader(pShader, nullptr, 0);

    //ID3D11ShaderResourceView *srv = nullptr;
    //if (auto texture = scene->GetTexture())
    //{
    //    srv = texture->GetShaderResourceView();
    //}
    //this->deviceContext->PSSetShaderResources(0, 1, &srv);
    //this->deviceContext->PSSetSamplers(0, 1, &samplerState);

    //this->deviceContext->DrawIndexed(static_cast<UINT>(mesh->GetNumIndicies()), 0, 0);
}

void Renderer::Present()
{
    this->swapChain->Present(0, 0);
}

bool Renderer::Resize(uint32_t width, uint32_t height) {
    if (width <= 0 || height <= 0)
        return false;

    LogBlank();
    LogInfo("Resizing window...");
    LogIndent();

    SafeRelease(this->renderTargetView);

    this->deviceContext->ClearState();
    this->deviceContext->Flush();

    HRESULT result = this->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(result)) {
        LogError("Failed to resize swap chain buffers");
        return false;
    }

    if (!this->CreateRenderTargetView())
        return false;

    // this->SetViewport(width, height);
    this->width = width;
    this->height = height;

    // this->frameGraph.OnResize(width, height);

    LogUnindent();
    return true;
}

void Renderer::AddView(const RenderView &view) {
    if (view.type == ViewType::PRIMARY) {
        for (RenderView &other : this->views) {
            if (other.type == ViewType::PRIMARY) {
                other = view;
                return;
            }
        }
    }

    this->views.push_back(view);
}