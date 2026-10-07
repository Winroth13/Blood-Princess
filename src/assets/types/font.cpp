#include "font.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "imgui/imgui.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"

#include <algorithm>

#undef min
#undef max

namespace {
    constexpr uint32_t ATLAS_WIDTH = 1024;
    constexpr uint32_t ATLAS_MAX_HEIGHT = 4096;
    constexpr uint32_t GLYPH_SPACING = 1;
    constexpr uint32_t FALLBACK_CODEPOINT = '?';

    constexpr struct CodepointRange {
        uint32_t first;
        uint32_t last;
    } CODEPOINT_RANGES[] = {
        {0x20, 0x7E},
        {0xA0, 0xFF}
    };

    struct GlyphBitmap {
        uint32_t glyphIndex;
        unsigned char *pixels;
        int width;
        int height;
        uint32_t atlasX;
        uint32_t atlasY;
    };

    uint32_t PackGlyphs(std::vector<GlyphBitmap> &bitmaps) {
        std::vector<GlyphBitmap *> sorted;
        sorted.reserve(bitmaps.size());
        for (GlyphBitmap &bitmap : bitmaps)
            sorted.push_back(&bitmap);

        std::sort(
            sorted.begin(), 
            sorted.end(), 
            [](const GlyphBitmap *a, const GlyphBitmap *b) { return a->height > b->height; }
        );

        uint32_t x = GLYPH_SPACING;
        uint32_t y = GLYPH_SPACING;
        uint32_t rowHeight = 0;

        for (GlyphBitmap *bitmap : sorted) {
            const uint32_t width = static_cast<uint32_t>(bitmap->width);
            const uint32_t height = static_cast<uint32_t>(bitmap->height);

            if (x + width + GLYPH_SPACING > ATLAS_WIDTH) {
                x = GLYPH_SPACING;
                y += rowHeight + GLYPH_SPACING;
                rowHeight = 0;
            }

            bitmap->atlasX = x;
            bitmap->atlasY = y;

            x += width + GLYPH_SPACING;
            rowHeight = std::max(rowHeight, height);
        }

        const uint32_t usedHeight = y + rowHeight + GLYPH_SPACING;
        return usedHeight <= ATLAS_MAX_HEIGHT ? usedHeight : 0;
    }
}

Font::~Font() {
    SafeRelease(this->textureAtlasSRV);
    SafeRelease(this->textureAtlas);
}

bool Font::Create(ID3D11Device *device, std::vector<unsigned char> &&fontData, const std::string &name) {
    this->fontData = std::move(fontData);
    this->fontInfo = std::make_unique<stbtt_fontinfo>();

    const int fontOffset = stbtt_GetFontOffsetForIndex(this->fontData.data(), 0);
    if (fontOffset < 0 || !stbtt_InitFont(this->fontInfo.get(), this->fontData.data(), fontOffset)) {
        LogWarn("Invalid TTF font '%s'", name.c_str());
        return false;
    }

    this->scale = stbtt_ScaleForMappingEmToPixels(this->fontInfo.get(), BASE_SIZE);

    int ascentUnits = 0;
    int descentUnits = 0;
    int lineGapsUnits = 0;
    stbtt_GetFontVMetrics(this->fontInfo.get(), &ascentUnits, &descentUnits, &lineGapsUnits);

    this->ascent = ascentUnits * this->scale;
    this->descent = descentUnits * this->scale;
    this->lineGap = lineGapsUnits * this->scale;

    std::vector<GlyphBitmap> bitmaps;

    for (const CodepointRange &range : CODEPOINT_RANGES) {
        for (uint32_t codepoint = range.first; codepoint <= range.last; ++codepoint) {
            const int glyphIndex = stbtt_FindGlyphIndex(this->fontInfo.get(), static_cast<int>(codepoint));
            if (glyphIndex == 0)
                continue;

            int advanceUnits = 0;
            int leftSideBearing = 0;
            stbtt_GetGlyphHMetrics(this->fontInfo.get(), glyphIndex, &advanceUnits, &leftSideBearing);

            FontGlyph glyph{};
            glyph.codepoint = codepoint;
            glyph.advance = advanceUnits * this->scale;

            int width = 0;
            int height = 0;
            int xOffset = 0;
            int yOffset = 0;

            unsigned char *pixels = stbtt_GetGlyphSDF(
                this->fontInfo.get(), 
                this->scale, 
                glyphIndex, 
                SDF_PADDING, 
                static_cast<unsigned char>(SDF_ON_EDGE_VALUE), 
                SDF_PIXEL_DISTANCE_SCALE, 
                &width, 
                &height, 
                &xOffset, 
                &yOffset
            );

            const uint32_t index = static_cast<uint32_t>(this->glyphs.size());

            if (pixels) {
                glyph.xOffset = static_cast<float>(xOffset);
                glyph.yOffset = static_cast<float>(yOffset);
                glyph.width = static_cast<float>(width);
                glyph.height = static_cast<float>(height);

                bitmaps.push_back({index, pixels, width, height, 0, 0});
            }

            this->glyphIndices[codepoint] = index;
            this->glyphs.push_back(glyph);
        }
    }

    auto freeBitmapsFunc = [&bitmaps]() {
        for (GlyphBitmap &bitmap : bitmaps)
            stbtt_FreeSDF(bitmap.pixels, nullptr);
        bitmaps.clear();
    };

    if (this->glyphs.empty()) {
        LogWarn("'%s' has no supported characters", name.c_str());
        freeBitmapsFunc();
        return false;
    }

    auto fallbackIter = this->glyphIndices.find(FALLBACK_CODEPOINT);
    this->fallbackGlyphIndex = fallbackIter != this->glyphIndices.end() ? fallbackIter->second : 0;

    const uint32_t usedHeight = PackGlyphs(bitmaps);
    if (usedHeight == 0) {
        LogWarn("Glyphs don't fit in atlas (%s, %ux%x)", name.c_str(), ATLAS_WIDTH, ATLAS_MAX_HEIGHT);
        freeBitmapsFunc();
        return false;
    }

    this->atlasWidth = ATLAS_WIDTH;
    this->atlasHeight = 32;
    while (this->atlasHeight < usedHeight)
        this->atlasHeight *= 2;

    std::vector<unsigned char> atlasPixels(static_cast<size_t>(this->atlasWidth) * this->atlasHeight, 0);

    for (const GlyphBitmap &bitmap : bitmaps) {
        for (int row = 0; row < bitmap.height; ++row) {
            memcpy(
                &atlasPixels[(bitmap.atlasY + row) * this->atlasWidth + bitmap.atlasX], 
                &bitmap.pixels[row * bitmap.width], 
                bitmap.width
            );
        }

        FontGlyph &glyph = this->glyphs[bitmap.glyphIndex];
        glyph.u0 = static_cast<float>(bitmap.atlasX) / this->atlasWidth;
        glyph.v0 = static_cast<float>(bitmap.atlasY) / this->atlasHeight;
        glyph.u1 = static_cast<float>(bitmap.atlasX + bitmap.width) / this->atlasWidth;
        glyph.v1 = static_cast<float>(bitmap.atlasY + bitmap.height) / this->atlasHeight;
    }

    freeBitmapsFunc();

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = this->atlasWidth;
    desc.Height = this->atlasHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = atlasPixels.data();
    data.SysMemPitch = this->atlasWidth;

    HRESULT result = device->CreateTexture2D(&desc, &data, &this->textureAtlas);
    if (FAILED(result)) {
        LogWarn("Failed to create atlas texture for '%s'", name.c_str());
        return false;
    }

    result = device->CreateShaderResourceView(this->textureAtlas, nullptr, &this->textureAtlasSRV);
    if (FAILED(result)) {
        LogWarn("Failed to create atlas SRV for '%s'", name.c_str());
        return false;
    }

    RenderUtils::SetDebugName(this->textureAtlas, DebugCategory::TEXTURE, "Font Atlas", name);
    RenderUtils::SetDebugName(this->textureAtlasSRV, DebugCategory::TEXTURE, "Font Atlas", name, "SRV");

    LogInfo("Loaded font '%s' with %zu glyphs (%ux%u)", name.c_str(), this->glyphs.size(), this->atlasWidth, this->atlasHeight);

    return true;
}

const FontGlyph *Font::GetGlyph(uint32_t codepoint) const {
    auto iter = this->glyphIndices.find(codepoint);
    if (iter != this->glyphIndices.end())
        return &this->glyphs[iter->second];

    return this->glyphs.empty() ? nullptr : &this->glyphs[this->fallbackGlyphIndex];
}

float Font::GetKerning(uint32_t left, uint32_t right) const {
    if (!this->fontInfo)
        return 0.0f;

    const uint64_t key = (static_cast<uint64_t>(left) << 32) | right;

    auto iter = this->kerningCache.find(key);
    if (iter != this->kerningCache.end())
        return iter->second;

    const float kerning = stbtt_GetCodepointKernAdvance(this->fontInfo.get(), static_cast<int>(left), static_cast<int>(right)) * this->scale;
    return this->kerningCache[key] = kerning;
}

void Font::DrawImGui() const {
    ImGui::Text("i'm not writing this fuck you");
}
