#pragma once

#include "scene/component.hpp"

class UITextComponent : public Component {
    RegisterComponent(UITextComponent);

    // ExportProperty(DirectX::XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});

public:
    UITextComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(); }
};