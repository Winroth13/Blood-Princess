#pragma once

#include "scene/component.hpp"
#include "assets/types/font.hpp"

#include <string>
#include <memory>

class UITextComponent : public Component {
    RegisterComponent(UITextComponent);

    ExportProperty(std::string, text, "");
    ExportProperty(std::shared_ptr<Font>, font, nullptr);
    ExportPropertyRanged(float, fontSize, "px", 1.0f, 500.0f, 0.5f, 32.0f);
    ExportPropertyColor(DirectX::XMFLOAT4, colour, {0.0f, 0.0f, 0.0f, 1.0f});


public:
    UITextComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUIRender(); }

    const std::string &GetText() const { return this->text; }
    void SetText(const std::string &text) { this->text = text; }
};