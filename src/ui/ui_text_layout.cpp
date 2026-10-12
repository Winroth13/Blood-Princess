#include "ui_text_layout.hpp"
#include "assets/types/font.hpp"

#include <algorithm>

#undef min
#undef max

namespace {
    constexpr uint32_t REPLACEMENT_CHARACTER = 0xFFFD;

    float MeasureRun(const Font &font, const std::vector<uint32_t> &codepoints, size_t start, size_t end, float scale) {
        if (start >= end)
            return 0.0f;

        float width = font.GetGlyph(codepoints[start])->advance;

        for (size_t i = start + 1; i < end; ++i) {
            width += font.GetKerning(codepoints[i - 1], codepoints[i]);
            width += font.GetGlyph(codepoints[i])->advance;
        }

        return width * scale;
    }
}

namespace UI {
    void TextLayout::Build(const Font &font, const std::vector<uint32_t> &codepoints, const TextLayoutSettings &layoutSettings) {
        this->lines.clear();
        this->glyphs.clear();
        this->size = {0.0f, 0.0f};

        const float scale = layoutSettings.fontSize / Font::BASE_SIZE;
        const float maxWidth = layoutSettings.boxDimensions.x;
        const size_t count = codepoints.size();

        auto emitLineFunc = [&](size_t start, size_t end) {
            while (end > start && codepoints[end - 1] == ' ')
                --end;

            TextLine line{};
            line.firstGlyph = static_cast<uint32_t>(this->glyphs.size());

            float penPosition = 0.0f;
            for (size_t i = start; i < end; ++i) {
                if (i > start)
                    penPosition += font.GetKerning(codepoints[i - 1], codepoints[i]) * scale;

                const FontGlyph *glyph = font.GetGlyph(codepoints[i]);
                this->glyphs.push_back({glyph, penPosition});
                penPosition += glyph->advance * scale;
            }

            line.glyphCount = static_cast<uint32_t>(this->glyphs.size()) - line.firstGlyph;
            line.width = penPosition;
            this->lines.push_back(line);
        };

        size_t lineStart = 0;
        size_t i = 0;

        while (i < count) {
            if (codepoints[i] == '\n') {
                emitLineFunc(lineStart, i);
                lineStart = ++i;
                continue;
            }

            size_t wordEnd = i + 1;
            if (codepoints[i] != ' ')
                while (wordEnd < count && codepoints[wordEnd] != ' ' && codepoints[wordEnd] != '\n')
                    ++wordEnd;

            if (layoutSettings.shouldWrap && codepoints[i] != ' ' && MeasureRun(font, codepoints, lineStart, wordEnd, scale) > maxWidth) {
                bool hasContent = false;
                for (size_t j = lineStart; j < i; ++j)
                    hasContent |= codepoints[j] != ' ';

                if (hasContent) {
                    emitLineFunc(lineStart, i);
                    lineStart = i;
                    continue;
                }

                size_t breakAt = i + 1;
                while (breakAt < wordEnd && MeasureRun(font, codepoints, i, breakAt + 1, scale) <= maxWidth)
                    ++breakAt;

                if (breakAt < wordEnd) {
                    emitLineFunc(i, breakAt);
                    lineStart = i = breakAt;
                    continue;
                }
            }

            i = wordEnd;
        }

        emitLineFunc(lineStart, count);

        const float ascent = font.GetAscent() * scale;
        const float descent = font.GetDescent() * scale;
        const float lineHeight = (font.GetAscent() - font.GetDescent() + font.GetLineGap()) * scale * layoutSettings.lineSpacing;
        const float textHeight = (this->lines.size() - 1) * lineHeight + (ascent - descent);

        float top = 0.0f;
        if (layoutSettings.alignVertical == AlignVertical::CENTRE)
            top = (layoutSettings.boxDimensions.y - textHeight) / 2;
        else if (layoutSettings.alignVertical == AlignVertical::BOTTOM)
            top = layoutSettings.boxDimensions.y - textHeight;

        float widest = 0.0f;

        for (size_t lineIndex = 0; lineIndex < this->lines.size(); ++lineIndex) {
            TextLine &line = this->lines[lineIndex];

            float x = 0.0f;
            if (layoutSettings.alignHorizontal == AlignHorizontal::CENTRE)
                x = (layoutSettings.boxDimensions.x - line.width) / 2;
            else if (layoutSettings.alignHorizontal == AlignHorizontal::RIGHT)
                x = layoutSettings.boxDimensions.x - line.width;

            line.origin = {x, top + ascent + lineIndex * lineHeight};
            widest = std::max(widest, line.width);
        }

        this->size = {widest, count > 0 ? textHeight : 0.0f};
    }

    DirectX::XMFLOAT2 TextLayout::Measure(
        const Font &font,
        const std::string &text,
        float fontSize,
        float lineSpacing,
        float maxWidth
    ) {
        std::vector<uint32_t> codepoints;
        DecodeUTF8(text, codepoints);

        TextLayoutSettings layoutSettings{};
        layoutSettings.fontSize = fontSize;
        layoutSettings.lineSpacing = lineSpacing;
        layoutSettings.shouldWrap = maxWidth > 0.0f;
        layoutSettings.boxDimensions = {maxWidth, 0.0f};

        TextLayout layout;
        layout.Build(font, codepoints, layoutSettings);
        return layout.GetSize();
    }

    float TextLayout::GetSingleLineHeight(const Font &font, float fontSize) {
        return (font.GetAscent() - font.GetDescent()) * fontSize / Font::BASE_SIZE;
    }

    // Honestly idk how this works lol
    void TextLayout::DecodeUTF8(const std::string &text, std::vector<uint32_t> &outCodepoints) {
        outCodepoints.clear();
        outCodepoints.reserve(text.size());

        const unsigned char *bytes = reinterpret_cast<const unsigned char *>(text.data());
        const size_t length = text.size();

        for (size_t i = 0; i < length;) {
            const unsigned char lead = bytes[i];

            uint32_t codepoint = 0;
            size_t sequenceLength = 0;

            if (lead < 0x80) {
                codepoint = lead;
                sequenceLength = 1;
            }
            else if (lead >= 0xC2 && lead < 0xE0) {
                codepoint = lead & 0x1F;
                sequenceLength = 2;
            }
            else if (lead >= 0xE0 && lead < 0xF0) {
                codepoint = lead & 0x0F;
                sequenceLength = 3;
            }
            else if (lead >= 0xF0 && lead < 0xF5) {
                codepoint = lead & 0x07;
                sequenceLength = 4;
            }

            bool isValid = sequenceLength > 0 && i + sequenceLength <= length;

            for (size_t j = 1; isValid && j < sequenceLength; ++j) {
                if ((bytes[i + j] & 0xC0) != 0x80)
                    isValid = false;
                else
                    codepoint = (codepoint << 6) | (bytes[i + j] & 0x3F);
            }

            if (isValid) {
                const bool isOverlong = (sequenceLength == 3 && codepoint < 0x800) || (sequenceLength == 4 && codepoint < 0x10000);
                const bool isSurrogate = codepoint >= 0xD800 && codepoint <= 0xDFFF;

                if (isOverlong || isSurrogate || codepoint > 0x10FFFF)
                    isValid = false;
            }

            if (!isValid) {
                outCodepoints.push_back(REPLACEMENT_CHARACTER);
                ++i;
                continue;
            }

            i += sequenceLength;

            if (codepoint == '\r')
                continue;

            outCodepoints.push_back(codepoint == '\t' ? ' ' : codepoint);
        }
    }
}