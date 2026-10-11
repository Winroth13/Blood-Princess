#pragma once

#include "scene/component.hpp"
#include "assets/types/font.hpp"
#include "ui/ui_types.hpp"
#include "ui/ui_text_layout.hpp"

#include <string>
#include <memory>
#include <vector>

class UITextComponent : public Component {
    RegisterComponent(UITextComponent);

    ExportProperty(std::string, text, "");
    ExportProperty(std::shared_ptr<Font>, font, nullptr);
    ExportPropertyRanged(float, fontSize, "px", 1.0f, 500.0f, 0.5f, 32.0f);
    ExportPropertyColor(DirectX::XMFLOAT4, colour, {0.0f, 0.0f, 0.0f, 1.0f});

    UI::AlignHorizontal alignHorizontal = UI::AlignHorizontal::LEFT;
    ExportProperty(std::string, alignHorizontal_, "LEFT");
    ExportPropertyChanged(alignHorizontal_, OnAlignHorizontalChanged);
    void OnAlignHorizontalChanged();

    UI::AlignVertical alignVertical = UI::AlignVertical::TOP;
    ExportProperty(std::string, alignVertical_, "TOP");
    ExportPropertyChanged(alignVertical_, OnAlignVerticalChanged);
    void OnAlignVerticalChanged();

    ExportProperty(bool, shouldWrap, true);
    ExportPropertyRanged(float, lineSpacing, "x", 0.1f, 5.0f, 0.01f, 1.0f);

    ExportPropertyCategory("Outline");
    ExportPropertyRanged(float, outlineThickness, "px", 0.0f, 50.0f, 0.05f, 0.0f);
    ExportPropertyColor(DirectX::XMFLOAT4, outlineColour, { 0.0f, 0.0f, 0.0f, 1.0f });

    ExportPropertyCategory("Shadow");
    ExportPropertyColor(DirectX::XMFLOAT4, shadowColour, { 0.0f, 0.0f, 0.0f, 0.0f });
    ExportProperty(DirectX::XMFLOAT2, shadowOffset, { 2.0f, 2.0f });

    std::string cachedText;
    std::vector<uint32_t> codepoints;
    UI::TextLayout layout;
    UI::TextLayoutSettings cachedSettings{};
    const Font *cachedFont = nullptr;
    bool isLayoutValid = false;

    void UpdateLayout(const UI::TextLayoutSettings &layoutSettings);

    void EmitGlyphs(
        UI::DrawList &drawList,
        DirectX::XMFLOAT2 origin,
        float glyphScale,
        DirectX::XMFLOAT4 colour,
        DirectX::XMFLOAT4 outlineColour,
        float outlineWidth,
        float distanceScale
    );

public:
    static constexpr const char *DEFAULT_FONT_PATH = "assets/fonts/Roboto-Regular.ttf";

    UITextComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUIRender(); }

private:
    void OnStart(const AppContext &context) override;
    void UIRender(UI::DrawList &drawList) override;

public:
    DirectX::XMFLOAT2 GetPreferredSize();

    const std::string &GetText() const { return this->text; }
    void SetText(const std::string &text) { this->text = text; }

    std::shared_ptr<Font> GetFont() const { return this->font; }
    void SetFont(const std::shared_ptr<Font> &font) { this->font = font; }

    float GetFontSize() const { return this->fontSize; }
    void SetFontSize(float fontSize) { this->fontSize = fontSize; }

    const DirectX::XMFLOAT4 &GetColour() const { return this->colour; }
    void SetColour(const DirectX::XMFLOAT4 &colour) { this->colour = colour; }

    UI::AlignHorizontal GetAlignHorizontal() const { return this->alignHorizontal; }
    void SetAlignHorizontal(UI::AlignHorizontal alignHorizontal);

    UI::AlignVertical GetAlignVertical() const { return this->alignVertical; }
    void SetAlignVertical(UI::AlignVertical alignVertical);

    void SetAlignment(UI::AlignHorizontal alignHorizontal, UI::AlignVertical alignVertical);

    bool GetShouldWrap() const { return this->shouldWrap; }
    void SetShouldWrap(bool shouldWrap) { this->shouldWrap = shouldWrap; }

    float GetLineSpacing() const { return this->lineSpacing; }
    void SetLineSpacing(float lineSpacing) { this->lineSpacing = lineSpacing; }

    float GetOutlineThickness() const { return this->outlineThickness; }
    void SetOutlineThickness(float outlineThickness) { this->outlineThickness = outlineThickness; }

    DirectX::XMFLOAT4 GetOutlineColour() const { return this->outlineColour; }
    void SetOutlineColour(const DirectX::XMFLOAT4 &outlineColour) { this->outlineColour = outlineColour; }

    void SetOutline(float thickness, const DirectX::XMFLOAT4 &colour) { 
        this->outlineThickness = thickness; this->outlineColour = colour; 
    }

    DirectX::XMFLOAT4 GetShadowColour() const { return this->shadowColour; }
    void SetShadowColour(const DirectX::XMFLOAT4 &shadowColour) { this->shadowColour = shadowColour; }

    DirectX::XMFLOAT2 GetShadowOffset() const { return this->shadowOffset; }
    void SetShadowOffset(const DirectX::XMFLOAT2 &shadowOffset) { this->shadowOffset = shadowOffset; }

    void SetShadow(const DirectX::XMFLOAT2 &offset, const DirectX::XMFLOAT4 &colour) { 
        this->shadowOffset = offset; this->shadowColour = colour; 
    }
};