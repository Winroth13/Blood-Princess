#pragma once

#include "scene/component.hpp"

class UIButtonComponent : public Component {
    RegisterComponent(UIButtonComponent);

    // ExportProperty(DirectX::XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});

public:
    UIButtonComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(); }
};