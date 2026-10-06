#pragma once
#include "scene/component.hpp"
#include "input/action.hpp"
#include "input/input.hpp"

class TransformComponent;

class PlayerControllerComponent : public Component 
{
	RegisterComponent(PlayerControllerComponent);

	ExportProperty(std::string, groupName, "players");
	ExportProperty(float, moveSpeed, 10.0f);
	ExportProperty(XMFLOAT2, mouseSensitivity, {0.1f,0.1f});
	ExportProperty(float, pitch, 0.0f);
	ExportProperty(float, yaw, 0.0f);

	ExportProperty(bool, canUseDash, true);
	ExportProperty(bool, isDashing, false);
	ExportProperty(float, dashCooldownTimer, .01f);
	ExportProperty(float, dashTimer, 0.0f);
	ExportProperty(float, dashDistance, 3.0f);
	ExportProperty(float, dashDuration, 0.2f);

	XMVECTOR dashDirection;
	CollisionManager* collisionRef{ nullptr };

	TransformComponent *rootTransform = nullptr;

public:
	PlayerControllerComponent(Entity* owner, bool isActive) :Component(owner, isActive) { UsesUpdate(); }

private:
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void OnDestroy(const AppContext& context) override;
	void GetDashDirection(XMVECTOR &direction);
public:
	void SetRootTransform(TransformComponent *transform) { this->rootTransform = transform; }

	void HandleMovement(float deltaTime);
	void HandleCameraLook(float deltaTime);
	void HandleCombat(float deltaTime);

	void PerformLightAttack();
	void PerformHeavyAttack();
	void PerformDash(float deltaTime, float dashDistance, float dashDuration);


	float GetMoveSpeed() const { return moveSpeed; };
	void SetMoveSpeed(float moveSpeed) { this->moveSpeed = moveSpeed; };

	XMFLOAT2 GetMouseSensitivity() const { return this->mouseSensitivity; };
	void SetMouseSensitivity(XMFLOAT2 sensitivity) { this->mouseSensitivity = sensitivity; };

	float GetPitch() const { return this->pitch; };
	void SetPitch(float pitch) { this->pitch = pitch; };

	float GetYaw() const { return this->yaw; };
	void SetYaw(float yaw) { this->yaw = yaw; };

	float GetDashCooldownTimer() const { return this->dashCooldownTimer; };
	void SetDashCooldownTimer(float dodgeCooldownTimer) { this->dashCooldownTimer = dashCooldownTimer; };
};