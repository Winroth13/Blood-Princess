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

    LogInfo("Device and Swap Chain created");

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

    if (!this->backbuffer.Create(this->device, this->swapChain))
        return false;

    if (!this->CreateDepthStencilView())
        return false;

    this->viewport.TopLeftX = 0.0f;
    this->viewport.TopLeftY = 0.0f;
    this->viewport.Width = static_cast<float>(this->width);
    this->viewport.Height = static_cast<float>(this->height);
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;

    LogUnindent();

	return true;
}

void Renderer::Shutdown()
{
    LogInfo("Shutting down Renderer...");
    LogIndent();

    LogInfo("Releasing Depth Stencil View");
    SafeRelease(this->depthStencilView);

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

    RenderView *primary = RenderUtils::GetView(this->views, ViewType::PRIMARY);
    if (!primary)
        LogWarn("No primary view was found");

    scene->GatherRenderCommands(this->views);

    this->deviceContext->ClearRenderTargetView(this->backbuffer.GetView(), this->clearColor);
    this->deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11RenderTargetView *view = this->backbuffer.GetView();
    this->deviceContext->OMSetRenderTargets(1, &view, nullptr);

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

    const GeometryCommand &command = primary->queue.geometryCommands[0];

    // Update Matrix constant buffer with the scene's transform
    //XMMATRIX worldT = XMMatrixTranspose(XMLoadFloat4x4(&command.worldMatrix));
    //XMMATRIX viewProjT = XMMatrixTranspose(XMMatrixMultiply(XMLoadFloat4x4(&primary->viewMatrix), XMLoadFloat4x4(&primary->projectionMatrix)));

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

    //ID3D11Buffer *vBuffer = command.vertexBuffer;
    //UINT stride = sizeof(Vertex);
    //UINT offset = 0;
    //this->deviceContext->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
    //this->deviceContext->IASetIndexBuffer(command.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    //this->deviceContext->IASetInputLayout(inputLayout);
    //this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //this->deviceContext->VSSetShader(vShader, nullptr, 0);
    //this->deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    //this->deviceContext->PSSetShader(pShader, nullptr, 0);

    //ID3D11ShaderResourceView *srv = nullptr;
    //this->deviceContext->PSSetShaderResources(0, 1, &srv);
    //this->deviceContext->PSSetSamplers(0, 1, &samplerState);

    //this->deviceContext->DrawIndexed(command.indexCount, command.startIndex, 0);
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

    this->deviceContext->ClearState();
    this->deviceContext->Flush();

    HRESULT result = this->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(result)) {
        LogError("Failed to resize swap chain buffers");
        return false;
    }

    if (!this->backbuffer.Create(this->device, this->swapChain))
        return false;

    // this->SetViewport(width, height);
    this->width = width;
    this->height = height;

    this->viewport.Width = static_cast<float>(this->width);
    this->viewport.Height = static_cast<float>(this->height);

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