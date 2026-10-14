#pragma once

#include "scene/component.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <array>

class Model;

enum class GibType
{
	GOBLIN,
	OGRE,
	SKELETON
};

struct GibData
{
	std::shared_ptr<Model> model;
	float scale;
};

class GibsComponent : public Component
{
	RegisterComponent(GibsComponent);

	const size_t GENERIC_GIB_START = 0;
	const size_t GENERIC_GIB_END = 1;

	const size_t GOBLIN_RIBCAGE = 2;
	const size_t GOBLIN_HEAD = 3;

	std::array<std::shared_ptr<Model>, 4> gibModels = {nullptr};

private:
	const std::vector<GibData> GetGibModels(GibType type, uint32_t count);

public:
	GibsComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };

	void OnStart(const AppContext &context) override;

	void SpawnGibs(XMFLOAT3 position, uint32_t count, GibType type);
};