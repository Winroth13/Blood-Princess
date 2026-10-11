#pragma once

#include "ui/ui_types.hpp"

#include <vector>
#include <string>

class Font;
struct FontGlyph;

namespace UI {
    struct TextLayoutSettings {
        float fontSize = 32.0f;
        float lineSpacing = 1.0f;
        bool shouldWrap = false;
        DirectX::XMFLOAT2 boxDimensions = {0.0f, 0.0f};
        AlignHorizontal alignHorizontal = AlignHorizontal::LEFT;
        AlignVertical alignVertical = AlignVertical::TOP;

        bool operator==(const TextLayoutSettings &other) const {
            return (
                this->fontSize        == other.fontSize &&
                this->lineSpacing     == other.lineSpacing &&
                this->shouldWrap      == other.shouldWrap &&
                this->boxDimensions.x == other.boxDimensions.x &&
                this->boxDimensions.y == other.boxDimensions.y &&
                this->alignHorizontal == other.alignHorizontal &&
                this->alignVertical   == other.alignVertical
            );
        }
    };

    struct TextLayoutGlyph {
        const FontGlyph *glyph;
        float penPosition;
    };

    struct TextLine {
        DirectX::XMFLOAT2 origin;
        float width;
        uint32_t firstGlyph;
        uint32_t glyphCount;
    };

    class TextLayout {
        std::vector<TextLine> lines;
        std::vector<TextLayoutGlyph> glyphs;
        DirectX::XMFLOAT2 size = {0.0f, 0.0f};

    public:
        TextLayout() = default;

        void Build(const Font &font, const std::vector<uint32_t> &codepoints, const TextLayoutSettings &layoutSettings);

        const std::vector<TextLine> &GetLines() const { return this->lines; }
        const std::vector<TextLayoutGlyph> &GetGlyphs() const { return this->glyphs; }
        DirectX::XMFLOAT2 GetSize() const { return this->size; }

        static void DecodeUTF8(const std::string &text, std::vector<uint32_t> &outCodepoints);
    };
}