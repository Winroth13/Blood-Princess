#pragma once

#include "scene/component.hpp"
#include "assets/types/texture_cube.hpp"

class EnvironmentComponent : public Component {
	RegisterComponent(EnvironmentComponent);

	ExportProperty(DirectX::XMFLOAT4, hsvShift, { 0.0f, 1.0f, 1.0f, 0.0f });
	ExportPropertyColor(DirectX::XMFLOAT3, fogColour, {0.0469f, 0.0302f, 0.0302f});
	ExportPropertyRanged(float, fogDensity, "", 0.0f, 1.0f, 0.005f, 1.0f);
	ExportPropertyRanged(float, fogStartDepth, "m", 0.0f, 15.0f, 0.05f, 0.0f);
	ExportPropertyRanged(float, fogEndDepth, "m", 0.0f, 15.0f, 0.05f, 15.0f);
	ExportProperty(std::shared_ptr<TextureCube>, textureCube, nullptr);
	
public:
	EnvironmentComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesRender(); }

	void Render(const RenderView &view, RenderQueue &queue) override;

	void SetHsvShift(DirectX::XMFLOAT4 hsvShift) { this->hsvShift = hsvShift; }
	void SetFogColour(DirectX::XMFLOAT3 colour) { this->fogColour = colour; }
	void SetFogDensity(float density) { this->fogDensity = density; }
	void SetFogStartDepth(float depth) { this->fogStartDepth = depth; }
	void SetFogEndDepth(float depth) { this->fogEndDepth = depth; }
	void SetSkybox(std::shared_ptr<TextureCube> textureCube) { this->textureCube = textureCube;  }
};