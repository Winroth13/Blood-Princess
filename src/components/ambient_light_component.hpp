#pragma once

#include "scene/component.hpp"
#include "DirectXMath.h"

class AmbientLightComponent : public Component 
{
    RegisterComponent(AmbientLightComponent);

    ExportPropertyColor(DirectX::XMFLOAT3, colour, { 1.0f, 1.0f, 1.0f });
    ExportProperty(float, intensity, 0.3f);

public:
    AmbientLightComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); }

private:
    void OnStart(const AppContext& context) override {};
    void Update(const AppContext &context, float deltaTime) override {}
    void Render(const RenderView& view, RenderQueue& queue) override;
    void OnDestroy(const AppContext& context) override {};

public:
    const DirectX::XMFLOAT3 &GetColour() const { return this->colour; }
    void SetColour(const DirectX::XMFLOAT3 &colour) { this->colour = colour; }

    float GetIntensity() const { return this->intensity; }
    void SetIntensity(float intensity) { this->intensity = intensity; }
};