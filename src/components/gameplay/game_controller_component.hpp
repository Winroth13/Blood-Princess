#pragma once

#include "scene/component.hpp"

class GameControllerComponent : public Component {
	RegisterComponent(GameControllerComponent);

public:
	GameControllerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesLateUpdate(); }

	void LateUpdate(const AppContext &context, float deltaTime);
};