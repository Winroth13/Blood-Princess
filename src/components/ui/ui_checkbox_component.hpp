#pragma once

#include "scene/component.hpp"

class UICheckboxComponent : public Component {
    RegisterComponent(UICheckboxComponent);

    // ExportProperty(DirectX::XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});

public:
    UICheckboxComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}
};