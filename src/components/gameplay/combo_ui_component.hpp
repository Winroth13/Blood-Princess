#pragma once
#include "scene/component.hpp"
#include "components/gameplay/player_controller_component.hpp"

class UITextComponent;

class ComboUIComponent : public Component
{
	RegisterComponent(ComboUIComponent);

	uint32_t combo = 0;

	float timer = 0.0f;

	const float MAX_COMBO = 99;
	const float COMBO_BOUNCE_TIME = 0.3f;

	PlayerControllerComponent *playerController = nullptr;
	UITextComponent *xLabel = nullptr;
	UITextComponent *comboLabel = nullptr;

	float xFontSize = 0.0f;
	float comboSize = 0.0f;
	float comboBounceTimer = -1.0f;

private:
	void OnStart(const AppContext &context);
	void Update(const AppContext &context, float deltaTime);

public:
	ComboUIComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~ComboUIComponent() = default;

	void SetPlayerController(PlayerControllerComponent *playerController) { this->playerController = playerController; }
	void SetXLabel(UITextComponent *label) { this->xLabel = label; }
	void SetComboLabel(UITextComponent *label) { this->comboLabel = label; }
};