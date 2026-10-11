#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

using namespace DirectX;

namespace UI {
    struct Vertex {
        XMFLOAT2 position;
        XMFLOAT2 uv;
        uint32_t colour;

        uint32_t isText;
        uint32_t outlineColour;
        float outlineWidth;
        float distanceScale;
    };

    struct DrawCommand {
        ID3D11ShaderResourceView *textureSRV;
        uint32_t indexStart;
        uint32_t indexCount;
    };

    class DrawList {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<DrawCommand> drawCommands;

        void AddVertices(const Vertex (&quad)[4], ID3D11ShaderResourceView *textureSRV);

    public:
        DrawList() = default;

        void Clear();

        void AddQuad(
            XMFLOAT2 topLeft,
            XMFLOAT2 bottomRight,
            XMFLOAT2 uvTopLeft,
            XMFLOAT2 uvBottomRight,
            XMFLOAT4 colour,
            ID3D11ShaderResourceView *textureSRV = nullptr
        );

        void AddGlyphQuad(
            XMFLOAT2 topLeft,
            XMFLOAT2 bottomRight,
            XMFLOAT2 uvTopLeft,
            XMFLOAT2 uvBottomRight,
            XMFLOAT4 colour,
            XMFLOAT4 outlineColour,
            float outlineWidth,
            float distanceScale,
            ID3D11ShaderResourceView *atlasSRV
        );

        const std::vector<Vertex> &GetVertices() const { return this->vertices; }
        const std::vector<uint32_t> &GetIndices() const { return this->indices; }
        const std::vector<DrawCommand> &GetDrawCommands() const { return this->drawCommands; }

        bool IsEmpty() const { return this->drawCommands.empty(); }
    };
}