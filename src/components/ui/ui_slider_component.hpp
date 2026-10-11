#pragma once

#include "scene/component.hpp"

class UISliderComponent : public Component {
    RegisterComponent(UISliderComponent);

    // ExportProperty(DirectX::XMFLOAT2, anchorTopLeft, {0.0f, 0.0f});

public:
    UISliderComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}
};