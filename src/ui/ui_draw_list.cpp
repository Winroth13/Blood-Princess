#include "ui_draw_list.hpp"

#include <algorithm>
#include <cmath>

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

    void DrawList::AddSlicedQuad(
        XMFLOAT2 topLeft,
        XMFLOAT2 bottomRight,
        XMFLOAT2 uvTopLeft,
        XMFLOAT2 uvBottomRight,
        XMFLOAT4 border,
        XMFLOAT2 textureSize,
        XMFLOAT2 borderScale,
        XMFLOAT4 colour,
        ID3D11ShaderResourceView *textureSRV
    ) {
        if (textureSize.x <= 0.0f || textureSize.y <= 0.0f) {
            this->AddQuad(topLeft, bottomRight, uvTopLeft, uvBottomRight, colour, textureSRV);
            return;
        }

        const float width = bottomRight.x - topLeft.x;
        const float height = bottomRight.y - topLeft.y;

        float left = border.x * borderScale.x;
        float top = border.y * borderScale.y;
        float right = border.z * borderScale.x;
        float bottom = border.w * borderScale.y;

        if (left + right > width && left + right > 0.0f) {
            const float factor = width / (left + right);
            left *= factor;
            right *= factor;
        }

        if (top + bottom > height && top + bottom > 0.0f) {
            const float factor = height / (top + bottom);
            top *= factor;
            bottom *= factor;
        }

        const float xs[4] = {std::round(topLeft.x), std::round(topLeft.x + left), std::round(bottomRight.x - right), std::round(bottomRight.x)};
        const float ys[4] = {std::round(topLeft.y), std::round(topLeft.y + top), std::round(bottomRight.y - bottom), std::round(bottomRight.y)};

        const float us[4] = {uvTopLeft.x, uvTopLeft.x + border.x / textureSize.x, uvBottomRight.x - border.z / textureSize.x, uvBottomRight.x};
        const float vs[4] = {uvTopLeft.y, uvTopLeft.y + border.y / textureSize.y, uvBottomRight.y - border.w / textureSize.y, uvBottomRight.y};

        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column) {
                this->AddQuad(
                    {xs[column], ys[row]},
                    {xs[column + 1], ys[row + 1]},
                    {us[column], vs[row]},
                    {us[column + 1], vs[row + 1]},
                    colour,
                    textureSRV
                );
            }
        }
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