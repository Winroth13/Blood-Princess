#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

using namespace DirectX;

struct UIVertex {
    XMFLOAT2 position;
    XMFLOAT2 uv;
    uint32_t colour;

    UIVertex(XMFLOAT2 position, XMFLOAT2 uv, uint32_t colour)
        : position(position), uv(uv), colour(colour) {}
};

struct UIDrawCommand {
    ID3D11ShaderResourceView *textureSRV;
    uint32_t indexStart;
    uint32_t indexCount;
};

class UIDrawList {
    std::vector<UIVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<UIDrawCommand> drawCommands;

    void AddVertices(const UIVertex (&quad)[4], ID3D11ShaderResourceView *textureSRV);

public:
    UIDrawList() = default;

    void Clear();

    void AddQuad(
        XMFLOAT2 topLeft,
        XMFLOAT2 bottomRight,
        XMFLOAT2 uvTopLeft,
        XMFLOAT2 uvBottomRight,
        XMFLOAT4 colour,
        ID3D11ShaderResourceView *textureSRV = nullptr
    );

    const std::vector<UIVertex> &GetVertices() const { return this->vertices; }
    const std::vector<uint32_t> &GetIndices() const { return this->indices; }
    const std::vector<UIDrawCommand> &GetDrawCommands() const { return this->drawCommands; }

    bool IsEmpty() const { return this->drawCommands.empty(); }
};