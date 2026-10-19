#pragma once

#include "scene/component.hpp"
#include "assets/types/texture_cube.hpp"

class EnvironmentComponent : public Component {
	RegisterComponent(EnvironmentComponent);

	ExportProperty(DirectX::XMFLOAT4, hsvShift, { 0.0f, 1.0f, 1.0f, 0.0f });
	ExportProperty(std::shared_ptr<TextureCube>, textureCube, nullptr);

public:
	EnvironmentComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesRender(); }

	void SetHSVShift(DirectX::XMFLOAT4 hsvShift) { this->hsvShift = hsvShift; }

	void Render(const RenderView &view, RenderQueue &queue) override;

	void SetHsvShift(DirectX::XMFLOAT4 hsvShift) { this->hsvShift = hsvShift; }
	void SetSkybox(std::shared_ptr<TextureCube> textureCube) { this->textureCube = textureCube;  }
};