#pragma once
#include "scene/component.hpp"
#include "input/action.hpp"
#include "input/input.hpp"
#include "events/event_bus.hpp"

class TransformComponent;
class AnimatorComponent;
class HitboxComponent;
class CameraComponent;
class Sound;
class Health;
class HurtboxComponent;

struct AttackData
{
	std::string animationName; // The name of the animation to play

	float beginDamage; // How many seconds into the animation should it begin dealing damage
	float stopDamage; // How many seconds into the animation should it stop dealing damage

	float beginCombo; // How many seconds into the animation should it allow combo
	float endCombo; // How many seconds into the animation should it no longer allow combo

	float damage; // The attacks actual damage
};

class PlayerControllerComponent : public Component 
{
	RegisterComponent(PlayerControllerComponent);

	ExportProperty(std::string, groupName, "players");
	ExportProperty(float, moveSpeed, 5.5f);
	ExportProperty(XMFLOAT2, mouseSensitivity, {0.1f,0.1f});
	ExportProperty(float, pitch, 0.0f);
	ExportProperty(float, yaw, 0.0f);

	ExportProperty(bool, canUseDash, true);
	ExportProperty(bool, isDashing, false);
	ExportProperty(float, dashCooldownTimer, .01f);
	ExportProperty(float, dashTimer, 0.0f);
	ExportProperty(float, dashDistance, 3.0f);
	ExportProperty(float, dashDuration, 0.2f);
	ExportProperty(float, lingeringDashDuration, 0.2f);
	ExportProperty(float, lingeringDashTimer, -1.0f);
	ExportProperty(float, maxDashAlpha, 1.0f);
	ExportProperty(float, maxDashDensity, 0.45f);
	ExportProperty(float, minDashDensity, 0.4f);
	ExportPropertyAngle(float, dashFov, DirectX::XMConvertToRadians(95.0f));

	// health
	ExportProperty(float, maxHealth, 200.0f);


	XMVECTOR dashDirection = {};
	CollisionManager* collisionRef = nullptr;

	TransformComponent *rootTransform = nullptr;
	// health
	Health* health = nullptr;
	HurtboxComponent* hurtbox = nullptr;
	bool isDead = false;
	bool isParrying = false;

	/*
	*	Sounds
	*/
	std::shared_ptr<Sound> attackShoutSound = nullptr;

	/*
	*	Attacks
	*/
	const uint32_t numLightAttacks = 2;
	const uint32_t numHeavyAttacks = 1;

	uint32_t currentAttack = -1;
	uint32_t combo = 0;
	float attackTimer = 0.0f;
	bool hasActivatedHitboxThisAttack = false;

	const float HIT_STOP_TIME = 0.06f;
	const float HIT_STOP_FACTOR = 0.25f;
	float hitStopTimer = -1.0f;

	const float ATTACK_COUNTER_RESET_TIME = 2.0f;

	uint32_t attackCounter = 0;
	float attackCounterTimer = -1.0;

	std::array<AttackData, 3> attacks =
	{
		/* Light Attacks */
		AttackData(
			"light_attack_01",
			0.05f, 0.5f,
			0.32f, 0.75f,
			50.0f
		),
			AttackData(
			"light_attack_02",
			0.05f, 0.65f,
			0.5f, 1.0f,
			50.0f
		),

		/* Heavy Attacks */
		AttackData(
			"heavy_attack_01",
			0.7f, 1.2f,
			1.64f, 3.0f,
			100.0f
		)
	};

public:
	PlayerControllerComponent(Entity* owner, bool isActive) :Component(owner, isActive) { UsesUpdate(); }

private:
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void OnDestroy(const AppContext& context) override;
	void GetDashDirection(XMVECTOR &direction);


	ConnectionId attackHitConnection = 0;
	ConnectionId damageTakenConnection = 0;

	void OnDeath();
	void OnAttackHit(const struct AttackHitEvent& event);
	void OnDamageTaken(const DamageTakenEvent& event);
public:
	AnimatorComponent *viewmodelAnimator = nullptr;
	HitboxComponent *hitbox = nullptr;
	CameraComponent *camera = nullptr;

	void SetRootTransform(TransformComponent *transform) { this->rootTransform = transform; }

	void HandleMovement(float deltaTime);
	void HandleCameraLook(float deltaTime);
	void HandleCombat(float deltaTime);

	void PerformLightAttack();
	void PerformAttack(AttackData &attackData);

	bool CanAttack();
	void HandleAttacks(float deltaTime);

	void PerformHeavyAttack();
	void PerformDash(float deltaTime, float dashDistance, float dashDuration);

	void PerformParry();
	bool IsParrying() const { return this->isParrying; }

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