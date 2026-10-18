#pragma once

#include "scene/component.hpp"
#include "assets/types/texture_cube.hpp"

class EnvironmentComponent : public Component {
	RegisterComponent(EnvironmentComponent);

	ExportProperty(DirectX::XMFLOAT4, hsvShift, { 0.0f, 1.0f, 1.0f, 0.0f });
	ExportProperty(std::shared_ptr<TextureCube>, textureCube, nullptr);

public:
	EnvironmentComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesRender(); }

	void Render(const RenderView &view, RenderQueue &queue) override;
};