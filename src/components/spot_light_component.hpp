#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"

class SpotLightComponent : public Component
{
	RegisterComponent(SpotLightComponent);

	ExportPropertyColor(DirectX::XMFLOAT3, colour, { 1.0f, 1.0f, 1.0f });
	ExportPropertyRanged(float, range, "m", 0.0f, 10.0f, 0.01f, 4.0f);
	ExportProperty(float, intensity, 1.0f);
	ExportPropertyClampedAngle(float, innerAngle, 0.0f, DirectX::XM_PI, DirectX::XM_PIDIV4);
	ExportPropertyClampedAngle(float, outerAngle, 0.0f, DirectX::XM_PI, DirectX::XM_PIDIV2);
	ExportProperty(bool, castShadows, true);

    DirectX::XMFLOAT4X4 viewProjectionMatrix{};

public:
	SpotLightComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); }

private:
	void LateUpdate(const AppContext &context, float deltaTime) override;
	void Render(const RenderView& view, RenderQueue& queue) override;

public:
    const DirectX::XMFLOAT3 &GetColour() const { return this->colour; }
    void SetColor(DirectX::XMFLOAT3 colour) { this->colour = colour; }

    float GetRange() const { return this->range; }
    void SetRange(float range) { this->range = range; }

    float GetIntensity() const { return this->intensity; }
    void SetIntensity(float intensity) { this->intensity = intensity; }

	float GetInnerAngle() const { return this->innerAngle; }
	void SetInnerAngle(float angle) { this->innerAngle = angle; }
	float GetOuterAngle() const { return this->outerAngle; }
	void SetOuterAngle(float angle) { this->outerAngle = angle; }

    bool GetCastShadows() const { return this->castShadows; }
    void SetCastShadows(bool castShadows) { this->castShadows = castShadows; }
};