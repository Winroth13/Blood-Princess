#pragma once
#include "scene/component.hpp"

class WaveController : public Component
{
	RegisterComponent(WaveController);

private:
	void OnStart(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override;

public:
	WaveController(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~WaveController() = default;

};