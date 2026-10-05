#pragma once

#include "scene/component.hpp"

class UIBarComponent : public Component {
    RegisterComponent(UIBarComponent);

    // ExportProperty(DirectX::XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});

public:
    UIBarComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(); }
};