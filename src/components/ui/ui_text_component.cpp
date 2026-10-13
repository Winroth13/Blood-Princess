#include "ui_text_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/font.hpp"
#include "scene/entity.hpp"
#include "ui/ui_draw_list.hpp"

#include <algorithm>
#include <cmath>

#undef min
#undef max

using namespace DirectX;

void UITextComponent::OnAlignHorizontalChanged() {
    if (this->alignHorizontal_ == "LEFT")
        this->alignHorizontal = UI::AlignHorizontal::LEFT;
    else if (this->alignHorizontal_ == "CENTRE")
        this->alignHorizontal = UI::AlignHorizontal::CENTRE;
    else if (this->alignHorizontal_ == "RIGHT")
        this->alignHorizontal = UI::AlignHorizontal::RIGHT;
}

void UITextComponent::OnAlignVerticalChanged() {
    if (this->alignVertical_ == "TOP")
        this->alignVertical = UI::AlignVertical::TOP;
    else if (this->alignVertical_ == "CENTRE")
        this->alignVertical = UI::AlignVertical::CENTRE;
    else if (this->alignVertical_ == "BOTTOM")
        this->alignVertical = UI::AlignVertical::BOTTOM;
}

void UITextComponent::UpdateLayout(const UI::TextLayoutSettings &layoutSettings) {
    const bool hasTextChanged = !this->isLayoutValid || this->text != this->cachedText;

    if (hasTextChanged) {
        this->cachedText = this->text;
        UI::TextLayout::DecodeUTF8(this->text, this->codepoints);
    }

    if (hasTextChanged || this->cachedFont != this->font.get() || layoutSettings != this->cachedSettings) {
        this->layout.Build(*this->font, this->codepoints, layoutSettings);
        this->cachedSettings = layoutSettings;
        this->cachedFont = this->font.get();
        this->isLayoutValid = true;
    }
}

void UITextComponent::EmitGlyphs(
    UI::DrawList &drawList,
    DirectX::XMFLOAT2 origin,
    float glyphScale,
    DirectX::XMFLOAT4 colour,
    DirectX::XMFLOAT4 outlineColour,
    float outlineWidth,
    float distanceScale
) {
    ID3D11ShaderResourceView *atlasSRV = this->font->GetTextureAtlasSRV();
    const std::vector<UI::TextLayoutGlyph> &glyphs = this->layout.GetGlyphs();

    for (const UI::TextLine &line : this->layout.GetLines()) {
        const float lineX = std::round(origin.x + line.origin.x);
        const float baseline = std::round(origin.y + line.origin.y);

        for (uint32_t i = line.firstGlyph; i < line.firstGlyph + line.glyphCount; ++i) {
            const FontGlyph *glyph = glyphs[i].glyph;
            if (glyph->width <= 0.0f)
                continue;

            const XMFLOAT2 topLeft = { lineX + glyphs[i].penPosition + glyph->xOffset * glyphScale, baseline + glyph->yOffset * glyphScale };
            const XMFLOAT2 bottomRight = { topLeft.x + glyph->width * glyphScale, topLeft.y + glyph->height * glyphScale };

            drawList.AddGlyphQuad(
                topLeft, 
                bottomRight,
                {glyph->u0, glyph->v0}, 
                {glyph->u1, glyph->v1},
                colour, 
                outlineColour, 
                outlineWidth, 
                distanceScale,
                atlasSRV
            );
        }
    }
}

void UITextComponent::OnStart(const AppContext &context) {
    if (!this->font)
        this->font = context.assetManager->Get<Font>(DEFAULT_FONT_PATH);
}

void UITextComponent::UIRender(UI::DrawList &drawList) {
    UIRectComponent *rectComponent = this->GetOwner()->GetComponent<UIRectComponent>();
    if (!rectComponent)
        return;

    const UI::Rect &rect = rectComponent->GetScreenRect();
    const XMFLOAT2 pixelScale = rectComponent->GetPixelScale();

    if (this->isRaycastTarget)
        drawList.AddHitTarget(rect, this->GetOwner());

    if (!this->font || !this->font->GetTextureAtlasSRV() || this->text.empty())
        return;

    UI::TextLayoutSettings layoutSettings{};
    layoutSettings.fontSize = this->fontSize * pixelScale.y;
    layoutSettings.lineSpacing = this->lineSpacing;
    layoutSettings.shouldWrap = this->shouldWrap;
    layoutSettings.boxDimensions = rect.size;
    layoutSettings.alignHorizontal = this->alignHorizontal;
    layoutSettings.alignVertical = this->alignVertical;

    if (layoutSettings.fontSize <= 0.0f)
        return;

    this->UpdateLayout(layoutSettings);

    const float glyphScale = layoutSettings.fontSize / Font::BASE_SIZE;
    const float distanceScale = 255.0f / Font::SDF_PIXEL_DISTANCE_SCALE * glyphScale;

    const float maxOutline = std::max(0.0f, Font::SDF_PADDING * glyphScale - 1.0f);
    const float outlineWidth = std::clamp(this->outlineThickness * pixelScale.y, 0.0f, maxOutline);

    const XMFLOAT4 outline = outlineWidth > 0.0f ? this->outlineColour : XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f };

    if (this->shadowColour.w > 0.0f) {
        const XMFLOAT2 shadowOrigin = { 
            rect.topLeft.x + this->shadowOffset.x * pixelScale.x, 
            rect.topLeft.y + this->shadowOffset.y * pixelScale.y 
        };
        const XMFLOAT4 shadowFill = { this->shadowColour.x, this->shadowColour.y, this->shadowColour.z, 0.0f };

        this->EmitGlyphs(
            drawList, 
            shadowOrigin, 
            glyphScale,
            shadowFill, 
            this->shadowColour,
            outlineWidth, 
            distanceScale
        );
    }

    if (this->colour.w <= 0.0f && outline.w <= 0.0f)
        return;

    this->EmitGlyphs(
        drawList, 
        rect.topLeft, 
        glyphScale,
        this->colour, 
        outline,
        outlineWidth, 
        distanceScale
    );
}

DirectX::XMFLOAT2 UITextComponent::GetPreferredSize() {
    return this->GetPreferredSize(0.0f);
}

DirectX::XMFLOAT2 UITextComponent::GetPreferredSize(float maxWidth) {
    if (!this->font)
        return {0.0f, 0.0f};

    return UI::TextLayout::Measure(*this->font, this->text, this->fontSize, this->lineSpacing, this->shouldWrap ? maxWidth : 0.0f);
}

void UITextComponent::SetAlignHorizontal(UI::AlignHorizontal alignHorizontal) {
    this->alignHorizontal = alignHorizontal;

    if (alignHorizontal == UI::AlignHorizontal::LEFT)
        this->alignHorizontal_ = "LEFT";
    else if (alignHorizontal == UI::AlignHorizontal::CENTRE)
        this->alignHorizontal_ = "CENTRE";
    else if (alignHorizontal == UI::AlignHorizontal::RIGHT)
        this->alignHorizontal_ = "RIGHT";
}

void UITextComponent::SetAlignVertical(UI::AlignVertical alignVertical) {
    this->alignVertical = alignVertical;

    if (alignVertical == UI::AlignVertical::TOP)
        this->alignVertical_ = "TOP";
    else if (alignVertical == UI::AlignVertical::CENTRE)
        this->alignVertical_ = "CENTRE";
    else if (alignVertical == UI::AlignVertical::BOTTOM)
        this->alignVertical_ = "BOTTOM";
}

void UITextComponent::SetAlignment(UI::AlignHorizontal alignHorizontal, UI::AlignVertical alignVertical) {
    this->SetAlignHorizontal(alignHorizontal);
    this->SetAlignVertical(alignVertical);
}