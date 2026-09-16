#pragma once
#include "scene/component.hpp"
#include "components/transform_component.hpp"
#include "DirectXMath.h"

class DirectionalLightComponent : public Component 
{
	RegisterComponent(DirectionalLightComponent);

	ExportPropertyColor(DirectX::XMFLOAT3, color, { 1.0f,1.0f,1.0f });
	ExportProperty(float, intensity,1.0f);
	ExportProperty(bool, ProduceShadow, false);

public:
	DirectionalLightComponent(Entity* owner, bool isActive);

	void OnStart(const AppContext& context) override {};
	void Update(const AppContext& context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override;
	void OnDestroy(const AppContext& context) override {};

	DirectX::XMFLOAT3 GetColor();
	bool SetColor(DirectX::XMFLOAT3 color);

	float GetIntensity();
	bool SetIntensity(float intensity);

	bool GeneratingShadow();
	bool SetGeneratingShadow(bool state);
};