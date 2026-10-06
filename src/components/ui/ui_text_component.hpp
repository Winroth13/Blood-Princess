#pragma once

#include "scene/component.hpp"

#include <string>

class UITextComponent : public Component {
    RegisterComponent(UITextComponent);

    ExportProperty(std::string, text, "");

public:
    UITextComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(), UsesUIRender(); }

    const std::string &GetText() const { return this->text; }
    void SetText(const std::string &text) { this->text = text; }
};