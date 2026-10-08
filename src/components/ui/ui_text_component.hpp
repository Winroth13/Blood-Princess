#pragma once

#include "scene/component.hpp"
#include "assets/types/font.hpp"
#include "ui/ui_types.hpp"

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

    UI::AlignVertical alignVertical = UI::AlignVertical::TOP;
    ExportProperty(std::string, alignVertical_, "TOP");
    ExportPropertyChanged(alignVertical_, OnAlignVerticalChanged);

    ExportProperty(bool, shouldWrap, true);
    ExportPropertyRanged(float, lineSpacing, "x", 0.1f, 5.0f, 0.01f, 1.0f);

    ExportPropertyCategory("Outline");
    ExportPropertyCategory("Shadow");

    std::string cachedText;
    std::vector<uint32_t> codepoints;

    void OnAlignHorizontalChanged();
    void OnAlignVerticalChanged();

public:
    UITextComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUIRender(); }

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

    bool GetShouldWrap() const { return this->shouldWrap; }
    void SetShouldWrap(bool shouldWrap) { this->shouldWrap = shouldWrap; }

    float GetLineSpacing() const { return this->lineSpacing; }
    void SetLineSpacing(float lineSpacing) { this->lineSpacing = lineSpacing; }
};