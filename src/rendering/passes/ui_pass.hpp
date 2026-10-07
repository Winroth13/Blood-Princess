#pragma once
#include "rendering/render_view.hpp"
#include "ui/ui_context.hpp"

#include <memory>
#include <vector>

struct ID3D11InputLayout;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

class VertexShader;
class PixelShader;
class AssetManager;
class SharedResources;
class BackBuffer;

class Texture2D;

class UIPass {
    ID3D11InputLayout *inputLayout = nullptr;

    ID3D11Buffer *vertexBuffer = nullptr;
    ID3D11Buffer *indexBuffer = nullptr;

    ID3D11RasterizerState *disableDepthRasterizerState = nullptr;
    ID3D11DepthStencilState *disableDepthStencilState = nullptr;
    ID3D11BlendState *blendState = nullptr;

    std::shared_ptr<VertexShader> vertexShader = nullptr;
    std::shared_ptr<PixelShader> pixelShader = nullptr;

    std::shared_ptr<Texture2D> whiteTexture = nullptr;

    RenderView view{};

    void CreateView(uint32_t width, uint32_t height);
    bool CreateBuffers(ID3D11Device *device);
    bool CreateInputLayout(ID3D11Device *device, VertexShader *vertexShader);
    bool CreateDisableDepthResources(ID3D11Device *device);

public:
    UIPass() = default;
    ~UIPass() = default;

    void Render(
        ID3D11Device *device,
        ID3D11DeviceContext *deviceContext,
        SharedResources &sharedResources,
        BackBuffer &backBuffer,
        UIContext &uiContext
    );

    bool Initialize(
        ID3D11Device *device,
        AssetManager *assetManager,
        uint32_t width,
        uint32_t height
    );

    bool Resize(uint32_t width, uint32_t height);

    void Shutdown();
};