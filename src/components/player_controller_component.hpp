#pragma once
#include "scene/component.hpp"

class PlayerControllerComponent : public Component 
{
	RegisterComponent(PlayerControllerComponent);
	ExportProperty(float, moveSpeed, 1.0f);
	ExportProperty(float, mouseSensitivity, 1.0f);
	ExportProperty(float, pitch, 1.0f);
	ExportProperty(float, yaw, 1.0f);

	ExportProperty(bool, canUseDodge, true);
	ExportProperty(float, dodgeCooldownTimer, 3.0f);
public:
	PlayerControllerComponent(Entity* owner, bool isActive) :Component(owner, isActive) { NoRender(); }

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;

	void HandleMovement(float deltaTime);
	void HandleCameraLook(float deltaTime);

	void PerformLightAttack();
	void PerformHeavyAttack();
	void PerformDodge();

	float GetMoveSpeed() const { return moveSpeed; };
	void SetMoveSpeed(float moveSpeed) { this->moveSpeed = moveSpeed; };

	float GetMouseSensitivity() const { return this->mouseSensitivity; };
	void SetMouseSensitivity(float sensitivity) { this->mouseSensitivity = sensitivity; };

	float GetPitch() const { return this->pitch; };
	void SetPitch(float pitch) { this->pitch = pitch; };

	float GetYaw() const { return this->yaw; };
	void SetYaw(float yaw) { this->yaw = yaw; };

	float GetDodgeCooldownTimer() const { return this->dodgeCooldownTimer; };
	void SetDodgeCooldownTimer(float dodgeCooldownTimer) { this->dodgeCooldownTimer = dodgeCooldownTimer; };
};