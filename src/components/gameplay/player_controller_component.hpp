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
class AudioManager;
class EnemySpawnerComponent;
class EnemyDatabaseComponent;
class WaveController;
class AudioClip;

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

	ExportPropertyCategory("General");
	ExportProperty(std::string, groupName, "players");
	ExportProperty(float, moveSpeed, 5.5f);
	ExportProperty(XMFLOAT2, mouseSensitivity, {0.1f,0.1f});
	ExportProperty(float, pitch, 0.0f);
	ExportProperty(float, yaw, 0.0f);

	ExportPropertyCategory("Dashing");
	ExportProperty(bool, canUseDash, true);
	ExportProperty(bool, isDashing, false);
	ExportProperty(float, dashCooldownTimer, .01f);
    ExportProperty(float, dashCooldownDuration, 1.5f);
	ExportProperty(float, dashTimer, 0.0f);
	ExportProperty(float, dashDistance, 3.0f);
	ExportProperty(float, dashDuration, 0.2f);
	ExportProperty(float, lingeringDashDuration, 0.2f);
	ExportProperty(float, lingeringDashTimer, -1.0f);
	ExportProperty(float, maxDashAlpha, 1.0f);
	ExportProperty(float, maxDashDensity, 0.45f);
	ExportProperty(float, minDashDensity, 0.4f);
	ExportPropertyAngle(float, dashFov, DirectX::XMConvertToRadians(95.0f));

	ExportPropertyCategory("Parry");
	ExportProperty(float, parryWindow, 0.25f);

	ExportPropertyCategory("Damage Flash");
	ExportProperty(float, damageFlashDuration, 0.5f);
	ExportProperty(float, damageFlashLingeringTimer, -1.0f);
	ExportProperty(float, damageSoundDuration, 0.8f);
	ExportProperty(float, damageSoundReminingTime, -1.0f);
	ExportProperty(float, lowHealthThreshold, 0.3f);
	ExportProperty(float, lowHealthFlashIntensity, 0.6f);
	ExportProperty(float, lowHealthPulseSpeed, 6.0f);

	// health
	ExportProperty(float, maxHealth, 200);

	// Looking
	bool canRotate = true;

	// Combo
	uint32_t score = 0;
	uint32_t combo = 0;
	float comboRemainingTime = -1.0f;
	ExportProperty(float, comboDuration, 2.5f);

	EnemySpawnerComponent *enemySpawner = nullptr;
	EnemyDatabaseComponent *enemyDatabase = nullptr;
	WaveController *waveController = nullptr;

	// movement
	XMFLOAT3 velcoity = { 0.0f,0.0f,0.0f };

	XMVECTOR dashDirection = {};
	CollisionManager* collisionRef = nullptr;

	TransformComponent *rootTransform = nullptr;
	// health
	Health* health = nullptr;
	HurtboxComponent* hurtbox = nullptr;
	bool isDead = false;
	bool isParrying = false;
	bool hasParriedThisParry = false;

    float lowHealthPulseTimer = 0.0f;

	// Parry
	const float PARRY_COUNTER_RESET_TIME = 1.0f;
	float parryCountResetTimer = -1.0f;
	uint32_t parryCount = 0;

	const float PARRY_FOV = 92.0f;

	std::shared_ptr<Texture2D> sparkTexture = nullptr;
	std::shared_ptr<Texture2D> parryShockwaveTexture = nullptr;

	uint32_t lastParryAnim = -1;
	std::array<std::string, 2> parryAnimations = 
	{ 
		"parry_success_01", 
		"parry_success_02" 
	};

	/*
	*	Sounds
	*/
	std::shared_ptr<Sound> attackShoutSound = nullptr;
	std::shared_ptr<Sound> hurtSound = nullptr;
	std::shared_ptr<Sound> parryShoutSound = nullptr;
    std::shared_ptr<AudioClip> sfxDash = nullptr;

	/*
	*	Attacks
	*/
	const uint32_t numLightAttacks = 2;
	const uint32_t numHeavyAttacks = 1;

	uint32_t currentAttack = -1;
	uint32_t attackCombo = 0;
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

	// Slowmotion
	const float SLOW_MOTION_TIME = 0.5f;
	float slowMotionTimer = -1.0f;
	float slowMotionMin = 0.01f;

	Application *application = nullptr;
	AudioManager *audioManager = nullptr;

    Signal<float> onDashCooldownChanged;
	Signal<uint32_t> onScoreChanged;
	Signal<uint32_t> onComboChanged;
	Signal<uint32_t, uint32_t> onComboEnd;
	Signal<float> onComboRemainingTimeChanged;

	Signal<> onParry;

    bool isPaused = false;
    Signal<> onPaused;
    Signal<> onUnpaused;

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
	void OnHitBlocked(HitboxComponent* hitbox);
	bool ConnectComboSignals();
	void IncrementCombo();

	void EndCombo();

	void SpawnSpark(XMFLOAT3 position);

public:
	AnimatorComponent *viewmodelAnimator = nullptr;
	HitboxComponent *hitbox = nullptr;
	CameraComponent *camera = nullptr;

	void SetRootTransform(TransformComponent *transform) { this->rootTransform = transform; }
	void SetEnemySpawner(EnemySpawnerComponent *enemnySpawner) { this->enemySpawner = enemnySpawner; }
	void SetEnemyDatabase(EnemyDatabaseComponent *enemyDatabase) { this->enemyDatabase = enemyDatabase; }
	void SetWaveController(WaveController *waveController) { this->waveController = waveController; }

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

	XMFLOAT3 GetVelocity() const { return this->velcoity; }

	XMFLOAT2 GetMouseSensitivity() const { return this->mouseSensitivity; };
	void SetMouseSensitivity(XMFLOAT2 sensitivity) { this->mouseSensitivity = sensitivity; };

	float GetPitch() const { return this->pitch; };
	void SetPitch(float pitch) { this->pitch = pitch; };

	float GetYaw() const { return this->yaw; };
	void SetYaw(float yaw) { this->yaw = yaw; };

	float GetDashCooldownTimer() const { return this->dashCooldownTimer; };
	void SetDashCooldownTimer(float dodgeCooldownTimer) { this->dashCooldownTimer = dashCooldownTimer; };

    float GetDashCooldownDuration() const { return this->dashCooldownDuration; }
	uint32_t GetScore() const { return this->score; }
	uint32_t GetCombo() const { return this->combo; }
	float GetComboDuration() const { return this->comboDuration; }

    bool GetIsPaused() const { return this->isPaused; }
    void SetIsPaused(bool isPaused);

	void SetCanRotate(bool rotate) { this->canRotate = rotate; }

	bool IsLightAttack() const { return (this->currentAttack >= 0 && this->currentAttack < this->numLightAttacks); }
	bool IsHeavyAttack() const { return (this->currentAttack >= this->numLightAttacks && this->currentAttack < this->numLightAttacks + this->numHeavyAttacks); }

	Signal<float> &GetOnDashCooldownChanged() { return this->onDashCooldownChanged; }
	Signal<uint32_t> &GetOnScoreChanged() { return this->onScoreChanged; }
	Signal<uint32_t> &GetOnComboChanged() { return this->onComboChanged; }
	Signal<uint32_t, uint32_t> &GetOnComboEnd() { return this->onComboEnd; }
	Signal<float> &GetOnComboRemainingTimeChanged() { return this->onComboRemainingTimeChanged; }
	Signal<> &GetOnParry() { return this->onParry; }
    Signal<> &GetOnPaused() { return this->onPaused; }
    Signal<> &GetOnUnpaused() { return this->onUnpaused; }
};