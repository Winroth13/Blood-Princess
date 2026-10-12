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

    constexpr int SDF_OVERSAMPLING = 4;
    constexpr float DISTANCE_INFINITY = 1e20f;

    constexpr struct CodepointRange {
        uint32_t first;
        uint32_t last;
    } CODEPOINT_RANGES[] = {
        {0x20, 0x7E},
        {0xA0, 0xFF}
    };

    struct GlyphBitmap {
        uint32_t glyphIndex;
        std::vector<unsigned char> pixels;
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

    void DistanceTransform1D(const float *f, float *d, int *v, float *z, int n) {
        int k = 0;
        v[0] = 0;
        z[0] = -DISTANCE_INFINITY;
        z[1] = DISTANCE_INFINITY;

        for (int q = 1; q < n; ++q) {
            float s = ((f[q] + q * q) - (f[v[k]] + v[k] * v[k])) / (2 * q - 2 * v[k]);
            while (s <= z[k]) {
                --k;
                s = ((f[q] + q * q) - (f[v[k]] + v[k] * v[k])) / (2 * q - 2 * v[k]);
            }

            ++k;
            v[k] = q;
            z[k] = s;
            z[k + 1] = DISTANCE_INFINITY;
        }

        k = 0;
        for (int q = 0; q < n; ++q) {
            while (z[k + 1] < q)
                ++k;

            d[q] = (q - v[k]) * (q - v[k]) + f[v[k]];
        }
    }

    void DistanceTransform2D(std::vector<float> &grid, int width, int height) {
        const int size = std::max(width, height);
        std::vector<float> f(size);
        std::vector<float> d(size);
        std::vector<int> v(size);
        std::vector<float> z(size + 1);

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y)
                f[y] = grid[y * width + x];

            DistanceTransform1D(f.data(), d.data(), v.data(), z.data(), height);

            for (int y = 0; y < height; ++y)
                grid[y * width + x] = d[y];
        }

        for (int y = 0; y < height; ++y) {
            DistanceTransform1D(&grid[y * width], d.data(), v.data(), z.data(), width);
            std::copy(d.begin(), d.begin() + width, grid.begin() + y * width);
        }
    }

    bool CreateGlyphSDF(
        const stbtt_fontinfo *fontInfo, 
        int glyphIndex, 
        float scale,
        std::vector<unsigned char> &outPixels, 
        int &outWidth, 
        int &outHeight, 
        int &outXOffset, 
        int &outYOffset
    ) {
        if (stbtt_IsGlyphEmpty(fontInfo, glyphIndex))
            return false;

        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        stbtt_GetGlyphBitmapBox(fontInfo, glyphIndex, scale, scale, &x0, &y0, &x1, &y1);
        if (x1 <= x0 || y1 <= y0)
            return false;

        const int padding = Font::SDF_PADDING;
        const int samples = SDF_OVERSAMPLING;

        outWidth = x1 - x0 + 2 * padding;
        outHeight = y1 - y0 + 2 * padding;
        outXOffset = x0 - padding;
        outYOffset = y0 - padding;

        const int highWidth = outWidth * samples;
        const int highHeight = outHeight * samples;
        std::vector<unsigned char> coverage(static_cast<size_t>(highWidth) * highHeight, 0);

        int highX0 = 0, highY0 = 0, highX1 = 0, highY1 = 0;
        stbtt_GetGlyphBitmapBox(fontInfo, glyphIndex, scale * samples, scale * samples, &highX0, &highY0, &highX1, &highY1);

        const int offsetX = highX0 - outXOffset * samples;
        const int offsetY = highY0 - outYOffset * samples;
        const int renderWidth = std::min(highX1 - highX0, highWidth - offsetX);
        const int renderHeight = std::min(highY1 - highY0, highHeight - offsetY);
        if (offsetX < 0 || offsetY < 0 || renderWidth <= 0 || renderHeight <= 0)
            return false;

        stbtt_MakeGlyphBitmap(
            fontInfo, &coverage[static_cast<size_t>(offsetY) * highWidth + offsetX],
            renderWidth, renderHeight, highWidth, scale * samples, scale * samples, glyphIndex
        );

        std::vector<float> toInside(coverage.size());
        std::vector<float> toOutside(coverage.size());

        for (size_t i = 0; i < coverage.size(); ++i) {
            const bool isInside = coverage[i] >= 128;
            toInside[i] = isInside ? 0.0f : DISTANCE_INFINITY;
            toOutside[i] = isInside ? DISTANCE_INFINITY : 0.0f;
        }

        DistanceTransform2D(toInside, highWidth, highHeight);
        DistanceTransform2D(toOutside, highWidth, highHeight);

        outPixels.resize(static_cast<size_t>(outWidth) * outHeight);

        for (int y = 0; y < outHeight; ++y) {
            for (int x = 0; x < outWidth; ++x) {
                float sum = 0.0f;

                for (int sy = 0; sy < samples; ++sy) {
                    for (int sx = 0; sx < samples; ++sx) {
                        const size_t i = static_cast<size_t>(y * samples + sy) * highWidth + (x * samples + sx);
                        sum += coverage[i] >= 128 ? std::sqrt(toOutside[i]) - 0.5f : 0.5f - std::sqrt(toInside[i]);
                    }
                }

                const float distance = sum / (samples * samples) / samples;
                const float value = Font::SDF_ON_EDGE_VALUE + Font::SDF_PIXEL_DISTANCE_SCALE * distance;

                outPixels[static_cast<size_t>(y) * outWidth + x] = static_cast<unsigned char>(std::clamp(std::round(value), 0.0f, 255.0f));
            }
        }

        return true;
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
            std::vector<unsigned char> pixels;

            const uint32_t index = static_cast<uint32_t>(this->glyphs.size());

            if (CreateGlyphSDF(this->fontInfo.get(), glyphIndex, this->scale, pixels, width, height, xOffset, yOffset)) {
                glyph.xOffset = static_cast<float>(xOffset);
                glyph.yOffset = static_cast<float>(yOffset);
                glyph.width = static_cast<float>(width);
                glyph.height = static_cast<float>(height);

                bitmaps.push_back({ index, std::move(pixels), width, height, 0, 0 });
            }

            this->glyphIndices[codepoint] = index;
            this->glyphs.push_back(glyph);
        }
    }

    if (this->glyphs.empty()) {
        LogWarn("'%s' has no supported characters", name.c_str());
        return false;
    }

    auto fallbackIter = this->glyphIndices.find(FALLBACK_CODEPOINT);
    this->fallbackGlyphIndex = fallbackIter != this->glyphIndices.end() ? fallbackIter->second : 0;

    const uint32_t usedHeight = PackGlyphs(bitmaps);
    if (usedHeight == 0) {
        LogWarn("Glyphs don't fit in atlas (%s, %ux%x)", name.c_str(), ATLAS_WIDTH, ATLAS_MAX_HEIGHT);
        return false;
    }

    this->atlasWidth = ATLAS_WIDTH;
    this->atlasHeight = 64;
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
