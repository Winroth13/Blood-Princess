#pragma once
#include "scene/component.hpp"
#include "components/gameplay/player_controller_component.hpp"

class UITextComponent;

class CreditsComponent : public Component
{
	RegisterComponent(CreditsComponent);

	ExportProperty(float, scrollSpeed, 20.0f);
	float topHeight = 0;
	float bottomHeight = 0;
	float creditsHeight = 0;
	bool isFirstFrame = true;

private:
	void OnStart(const AppContext &context);
	void Update(const AppContext &context, float deltaTime) override;

public:
	CreditsComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~CreditsComponent() = default;

	void SetHeights(float topHeight, float bottomHeight) { this->topHeight = topHeight; this->bottomHeight = bottomHeight; }
};