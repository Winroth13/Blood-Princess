#include "ui_pass.hpp"
#include "rendering/render_utils.hpp"
#include "core/logging.hpp"
#include "assets/asset_manager.hpp"
#include "rendering/resources/backbuffer.hpp"
#include "rendering/shared_resources.hpp"
#include "ui/ui_context.hpp"
#include "ui/ui_draw_list.hpp"

namespace {
    constexpr size_t INITIAL_QUAD_CAPACITY = 4096;
}

void UIPass::CreateView(uint32_t width, uint32_t height)
{
    DirectX::XMStoreFloat4x4(
        &this->view.viewMatrix,
        DirectX::XMMatrixIdentity()
    );
    DirectX::XMStoreFloat4x4(
        &this->view.projectionMatrix,
        XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 0.1f, 2.0f)
    );

    this->view.cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    this->view.nearPlane = 0.0f;
    this->view.farPlane = 2.0f;
}

bool UIPass::CreateInputLayout(ID3D11Device *device, VertexShader *vertexShader)
{
    LogInfo("Creating Input Layout");
    LogIndent();

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION",         0, DXGI_FORMAT_R32G32_FLOAT,   0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",         0, DXGI_FORMAT_R32G32_FLOAT,   0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",	          0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "IS_TEXT",	      0, DXGI_FORMAT_R32_UINT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "OUTLINE_COLOUR",	  0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "OUTLINE_WIDTH",	  0, DXGI_FORMAT_R32_FLOAT,      0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "DISTANCE_SCALE",	  0, DXGI_FORMAT_R32_FLOAT,      0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    HRESULT result = device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc),
        vertexShader->GetByteCode().data(),
        vertexShader->GetByteCode().length(),
        &this->inputLayout
    );

    if (FAILED(result)) {
        LogError("Failed to create Input Layout");
        return false;
    }

    RenderUtils::SetDebugName(this->inputLayout, DebugCategory::RENDER_PASS, "Input Layout", "UI");

    LogUnindent();

    return true;
}

bool UIPass::CreateDisableDepthResources(ID3D11Device *device) {
    HRESULT result;

    {
        D3D11_DEPTH_STENCIL_DESC desc = {};
        desc.DepthEnable = false;

        result = device->CreateDepthStencilState(&desc, &this->disableDepthStencilState);

        if (FAILED(result)) {
            LogError("DirectX failed to create Depth Stencil");
            return false;
        }

        RenderUtils::SetDebugName(this->disableDepthStencilState, DebugCategory::RENDER_PASS, "DSS", "UI");
    }

    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.CullMode = D3D11_CULL_NONE;
        desc.FillMode = D3D11_FILL_SOLID;
        desc.DepthClipEnable = false;

        result = device->CreateRasterizerState(&desc, &this->disableDepthRasterizerState);

        if (FAILED(result)) {
            LogError("DirectX failed to create Rasterizer State");
            return false;
        }

        RenderUtils::SetDebugName(this->disableDepthRasterizerState, DebugCategory::RENDER_PASS, "RS", "UI");
    }

    {
        D3D11_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&blendDesc, &this->blendState);

        RenderUtils::SetDebugName(this->blendState, DebugCategory::RENDER_PASS, "BS", "UI");
    }

    return true;
}

bool UIPass::EnsureBufferCapacity(ID3D11Device *device, uint32_t vertexCount, uint32_t indexCount) {
    if (vertexCount > this->vertexCapacity || !this->vertexBuffer)
    {
        uint32_t capacity = this->vertexCapacity > 0 ? this->vertexCapacity : INITIAL_QUAD_CAPACITY * 4;
        while (capacity < vertexCount)
            capacity *= 2;

        SafeRelease(this->vertexBuffer);

        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.ByteWidth = sizeof(UI::Vertex) * capacity;

        if (FAILED(device->CreateBuffer(&desc, nullptr, &this->vertexBuffer)))
        {
            LogError("Failed to create UI Vertex Buffer");
            this->vertexCapacity = 0;
            return false;
        }

        RenderUtils::SetDebugName(this->vertexBuffer, DebugCategory::BUFFER, "Vertex Buffer", "UI");
        this->vertexCapacity = capacity;
    }

    if (indexCount > this->indexCapacity || !this->indexBuffer)
    {
        uint32_t capacity = this->indexCapacity > 0 ? this->indexCapacity : INITIAL_QUAD_CAPACITY * 6;
        while (capacity < indexCount)
            capacity *= 2;

        SafeRelease(this->indexBuffer);

        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.ByteWidth = sizeof(uint32_t) * capacity;

        if (FAILED(device->CreateBuffer(&desc, nullptr, &this->indexBuffer)))
        {
            LogError("Failed to create UI Index Buffer");
            this->indexCapacity = 0;
            return false;
        }

        RenderUtils::SetDebugName(this->indexBuffer, DebugCategory::BUFFER, "Index Buffer", "UI");
        this->indexCapacity = capacity;
    }

    return true;
}

void UIPass::Render(
    ID3D11Device *device,
    ID3D11DeviceContext *deviceContext,
    SharedResources &sharedResources,
    BackBuffer &backBuffer,
    UI::Context &uiContext
) {
    const UI::DrawList &drawList = uiContext.GetDrawList();
    if (drawList.IsEmpty())
        return;

    BeginAnnotation("UI Pass");

    const std::vector<UI::Vertex> &vertices = drawList.GetVertices();
    const std::vector<uint32_t> &indices = drawList.GetIndices();

    if (!this->EnsureBufferCapacity(device, static_cast<uint32_t>(vertices.size()), static_cast<uint32_t>(indices.size())))
    {
        EndAnnotation();
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (FAILED(deviceContext->Map(this->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        LogWarn("Failed to map the UI Vertex Buffer");
        EndAnnotation();
        return;
    }
    memcpy(mapped.pData, vertices.data(), sizeof(UI::Vertex) * vertices.size());
    deviceContext->Unmap(this->vertexBuffer, 0);

    if (FAILED(deviceContext->Map(this->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        LogWarn("Failed to map the UI Index Buffer");
        EndAnnotation();
        return;
    }
    memcpy(mapped.pData, indices.data(), sizeof(uint32_t) * indices.size());
    deviceContext->Unmap(this->indexBuffer, 0);

    sharedResources.UploadPerViewData(deviceContext, this->view);

    ID3D11RenderTargetView *rtv = backBuffer.GetRenderTargetView();
    deviceContext->OMSetRenderTargets(1, &rtv, nullptr);

    deviceContext->RSSetState(this->disableDepthRasterizerState);
    deviceContext->OMSetDepthStencilState(this->disableDepthStencilState, 0);
    deviceContext->OMSetBlendState(this->blendState, nullptr, 0xFFFFFFFF);

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(this->inputLayout);

    const UINT stride = sizeof(UI::Vertex);
    const UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(this->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
    deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

    ID3D11Buffer *perView = sharedResources.GetPerView();
    deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);

    sharedResources.BindSamplers(deviceContext);

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(backBuffer.GetWidth());
    viewport.Height = static_cast<float>(backBuffer.GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    ID3D11ShaderResourceView *whiteSRV = this->whiteTexture->GetShaderResourceView();
    for (const UI::DrawCommand &command : drawList.GetDrawCommands())
    {
        ID3D11ShaderResourceView *srv = command.textureSRV ? command.textureSRV : whiteSRV;
        deviceContext->PSSetShaderResources(0, 1, &srv);
        deviceContext->DrawIndexed(command.indexCount, command.indexStart, 0);
    }

    deviceContext->RSSetState(nullptr);
    deviceContext->OMSetDepthStencilState(nullptr, 0);
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    deviceContext->IASetInputLayout(nullptr);

    deviceContext->VSSetShader(nullptr, nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0);

    ID3D11ShaderResourceView *nullSRV = {};
    deviceContext->PSSetShaderResources(0, 1, &nullSRV);

    EndAnnotation();
}

bool UIPass::Initialize(
    ID3D11Device *device,
    AssetManager *assetManager,
    uint32_t width,
    uint32_t height
) {
    LogInfo("Initializing UI Pass...");
    LogIndent();

    this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_ui.cso");
    this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_ui.cso");

    this->whiteTexture = assetManager->GetDefaultTexture(DefaultTexture::ALBEDO);

    this->CreateView(width, height);

    if (!this->CreateInputLayout(device, this->vertexShader.get()))
        return false;

    if (!this->CreateDisableDepthResources(device))
        return false;

    if (!this->EnsureBufferCapacity(device, INITIAL_QUAD_CAPACITY * 4, INITIAL_QUAD_CAPACITY * 6))
        return false;

    LogUnindent();

    return true;
}

bool UIPass::Resize(uint32_t width, uint32_t height) {
    this->CreateView(width, height);
    return true;
}

void UIPass::Shutdown() {
    SafeRelease(this->disableDepthRasterizerState);
    SafeRelease(this->disableDepthStencilState);
    SafeRelease(this->blendState);

    SafeRelease(this->indexBuffer);
    SafeRelease(this->vertexBuffer);
    SafeRelease(this->inputLayout);

    this->whiteTexture.reset();
}
