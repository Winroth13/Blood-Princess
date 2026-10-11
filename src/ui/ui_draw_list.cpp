#include "ui_draw_list.hpp"

#include <algorithm>

static uint32_t PackColour(const XMFLOAT4 &colour) {
    auto func = [](float v) -> uint32_t { return static_cast<uint32_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
    return func(colour.x) | (func(colour.y) << 8) | (func(colour.z) << 16) | (func(colour.w) << 24);
}

namespace UI {
    void DrawList::AddVertices(const Vertex(&quad)[4], ID3D11ShaderResourceView *textureSRV) {
        const uint32_t baseIndex = static_cast<uint32_t>(this->vertices.size());
        this->vertices.insert(this->vertices.end(), std::begin(quad), std::end(quad));

        const uint32_t quadIndices[6] = {baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3};

        if (this->drawCommands.empty() || this->drawCommands.back().textureSRV != textureSRV)
            this->drawCommands.push_back({textureSRV, static_cast<uint32_t>(this->indices.size()), 0});

        this->indices.insert(this->indices.end(), std::begin(quadIndices), std::end(quadIndices));
        this->drawCommands.back().indexCount += 6;
    }

    void DrawList::Clear() {
        this->vertices.clear();
        this->indices.clear();
        this->drawCommands.clear();
    }

    void DrawList::AddQuad(
        XMFLOAT2 topLeft,
        XMFLOAT2 bottomRight,
        XMFLOAT2 uvTopLeft,
        XMFLOAT2 uvBottomRight,
        XMFLOAT4 colour,
        ID3D11ShaderResourceView *textureSRV
    ) {
        if (topLeft.x >= bottomRight.x || topLeft.y >= bottomRight.y)
            return;

        uint32_t packedColour = PackColour(colour);
        const Vertex quad[4] = {
            {{topLeft.x,     topLeft.y},     {uvTopLeft.x,     uvTopLeft.y},     packedColour, 0, 0x0, 0.0f, 0.0f},
            {{bottomRight.x, topLeft.y},     {uvBottomRight.x, uvTopLeft.y},     packedColour, 0, 0x0, 0.0f, 0.0f},
            {{bottomRight.x, bottomRight.y}, {uvBottomRight.x, uvBottomRight.y}, packedColour, 0, 0x0, 0.0f, 0.0f},
            {{topLeft.x,     bottomRight.y}, {uvTopLeft.x,     uvBottomRight.y}, packedColour, 0, 0x0, 0.0f, 0.0f}
        };

        this->AddVertices(quad, textureSRV);
    }

    void DrawList::AddGlyphQuad(
        XMFLOAT2 topLeft,
        XMFLOAT2 bottomRight,
        XMFLOAT2 uvTopLeft,
        XMFLOAT2 uvBottomRight,
        XMFLOAT4 colour,
        XMFLOAT4 outlineColour,
        float outlineWidth,
        float distanceScale,
        ID3D11ShaderResourceView *atlasSRV
    ) {
        if (topLeft.x >= bottomRight.x || topLeft.y >= bottomRight.y)
            return;

        uint32_t packedColour = PackColour(colour);
        uint32_t packedOutlineColour = PackColour(outlineColour);

        const Vertex quad[4] = {
            {{topLeft.x,     topLeft.y},     {uvTopLeft.x,     uvTopLeft.y},     packedColour, 1, packedOutlineColour, outlineWidth, distanceScale},
            {{bottomRight.x, topLeft.y},     {uvBottomRight.x, uvTopLeft.y},     packedColour, 1, packedOutlineColour, outlineWidth, distanceScale},
            {{bottomRight.x, bottomRight.y}, {uvBottomRight.x, uvBottomRight.y}, packedColour, 1, packedOutlineColour, outlineWidth, distanceScale},
            {{topLeft.x,     bottomRight.y}, {uvTopLeft.x,     uvBottomRight.y}, packedColour, 1, packedOutlineColour, outlineWidth, distanceScale}
        };

        this->AddVertices(quad, atlasSRV);
    }
}