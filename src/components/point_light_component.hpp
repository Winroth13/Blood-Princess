#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"

class PointLightComponent : public Component
{
	RegisterComponent(PointLightComponent);

	ExportPropertyColor(DirectX::XMFLOAT3, colour, { 1.0f,1.0f,1.0f });
	ExportProperty(float, attenuation, 1.0f);
	ExportProperty(float, intensity, 1.0f);
	ExportPropertyRanged(float, radius, "m", 0.0f, 10.0f, 0.01f, 4.0f);
	ExportProperty(bool, castShadows, false);

public:
    PointLightComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}

private:
	void OnStart(const AppContext& context) override {};
    void Update(const AppContext &context, float deltaTime) override;
	void Render(const RenderView& view, RenderQueue& queue) override;
	void OnDestroy(const AppContext& context) override {};

public:
    const DirectX::XMFLOAT3 &GetColour() const { return this->colour; }
    bool SetColour(DirectX::XMFLOAT3 colour) { this->colour = colour; }

    float GetIntensity() const { return this->intensity; }
    void SetIntensity(float intensity) { this->intensity = intensity; }

	float GetAttenuation() const { return this->attenuation; }
    void SetAttenuation(float attenuation) { this->attenuation = attenuation; }

	float GetRadius() const { return this->radius; }
    void SetRadius(float radius) { this->radius = radius; }

    bool GetCastsShadows() const { return this->castShadows; }
    void SetCastShadows(bool castShadows) { this->castShadows = castShadows; }
};

