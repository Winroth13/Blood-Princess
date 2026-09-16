#pragma once
#include "scene/component.hpp"
#include "components/transform_component.hpp"
#include "DirectXMath.h"

class SpotLightComponent : public Component
{
	RegisterComponent(SpotLightComponent);
	ExportPropertyColor(DirectX::XMFLOAT3, color, { 1.0f,1.0f,1.0f });
	ExportPropertyRanged(float, range, "m", 0.0f, 10.0f, 0.01f, 4.0f);
	ExportProperty(float, attenuation, 1.0f);
	ExportProperty(float, intensity, 1.0f);
	ExportPropertyClampedAngle(float, angle,0.0f, DirectX::XM_PI ,DirectX::XM_PI/4.0f);
	ExportProperty(bool, ProduceShadow, false);

public:
	SpotLightComponent(Entity* owner, bool isActive);

	void OnStart(const AppContext& context) override {};
	void Update(const AppContext& context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override;
	void OnDestroy(const AppContext& context) override {};

	DirectX::XMFLOAT3 GetColor();
	bool SetColor(DirectX::XMFLOAT3 color);

	float GetRange();
	bool SetRange(float range);

	float GetAttenuation();
	bool SetAttenuation(float attenuation);

	float GetIntensity();
	bool SetIntensity(float intensity);

	float GetAngle();
	bool SetAngle(float angle);

	bool GeneratingShadow();
	bool SetGeneratingShadow(bool state);
};