#pragma once

#include "assets/asset.hpp"

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

struct stbtt_fontinfo;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11Device;

struct FontGlyph {
    uint32_t codepoint = 0;

    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;

    float xOffset = 0.0f;
    float yOffset = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    float advance = 0.0f;
};

class Font : public Asset {
public:
    static constexpr float BASE_SIZE = 48.0f;
    static constexpr int SDF_PADDING = 8;
    static constexpr int SDF_ON_EDGE_VALUE = 128;
    static constexpr float SDF_PIXEL_DISTANCE_SCALE = static_cast<float>(SDF_ON_EDGE_VALUE) / SDF_PADDING;

private:
    std::vector<unsigned char> fontData;
    std::unique_ptr<stbtt_fontinfo> fontInfo;
    float scale = 1.0f;

    std::vector<FontGlyph> glyphs;
    std::unordered_map<uint32_t, uint32_t> glyphIndices;
    uint32_t fallbackGlyphIndex = 0;

    mutable std::unordered_map<uint64_t, float> kerningCache;

    float ascent = 0.0f;
    float descent = 0.0f;
    float lineGap = 0.0f;

    ID3D11Texture2D *textureAtlas = nullptr;
    ID3D11ShaderResourceView *textureAtlasSRV = nullptr;
    uint32_t atlasWidth = 0;
    uint32_t atlasHeight = 0;

public:
    Font() = default;
    ~Font();

    bool Create(ID3D11Device *device, std::vector<unsigned char> &&fontData, const std::string &name);

    const FontGlyph *GetGlyph(uint32_t codepoint) const;

    float GetKerning(uint32_t left, uint32_t right) const;

    float GetAscent() const { return this->ascent; }
    float GetDescent() const { return this->descent; }
    float GetLineGap() const { return this->lineGap; }

    ID3D11ShaderResourceView *GetTextureAtlasSRV() const { return this->textureAtlasSRV; }

    void DrawImGui() const;
};