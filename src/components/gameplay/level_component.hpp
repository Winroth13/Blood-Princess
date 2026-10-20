#pragma once
#include "scene/component.hpp"
#include "assets/types/floor.hpp"
#include "events/signal.hpp"

class LevelComponent : public Component
{
	RegisterComponent(LevelComponent);

private:
	void OnStart(const AppContext &context) override;

public:
	LevelComponent(Entity *owner, bool isActive) : Component(owner, isActive) { };
	~LevelComponent() = default;
};