#pragma once

#include "scene/component.hpp"
#include "ai/ai_manager.hpp"
#include "assets/types/enemy.hpp"

#include <DirectXMath.h>
#include <string>

class TransformComponent;
class AnimatorComponent;
class HitboxComponent;
class GibsComponent;
class SoundComponent;
class SplatterComponent;
class SpatialGrid;
class Sound;
class Enemy;

class AudioClip;
class AudioManager;
struct AttackHitEvent;

class EnemyControllerComponent : public Component
{
	RegisterComponent(EnemyControllerComponent);

	ExportProperty(std::string, groupName, "Enemy");

	ExportProperty(float, stopDistance, 1.0f);

	ExportProperty(std::shared_ptr<Sound>, hitSound, nullptr);
	ExportProperty(std::shared_ptr<Sound>, deathSound, nullptr);
	ExportProperty(std::shared_ptr<Sound>, attackHitSound, nullptr);
	ExportProperty(float, footstepIntervalMeters, 2.5f);
	ExportProperty(float, randomSoundMinInterval, 5.0f);
	ExportProperty(float, randomSoundMaxInterval, 50.0f);
	// attack
	ExportProperty(float, attackDamage, 15.0f);
	ExportProperty(float, attackKnockback, 2.0f);
	ExportProperty(float, attackHitboxDelay, 0.3f);
	ExportProperty(float, attackHitboxDuration, 0.3f);

private:
	TransformComponent *transform = nullptr;
	AnimatorComponent *animator = nullptr;
	Entity *hitboxEntity = nullptr;

	GibsComponent *gibs = nullptr;
	AIManager* aiManager = nullptr;
	AudioManager* audioManager = nullptr;
	SoundComponent *sound = nullptr;
	SplatterComponent *splatter = nullptr;

	XMFLOAT3 velocity = {0.0f, 0.0f, 0.0f};

	std::shared_ptr<AudioClip> bowDrawClip;
	std::shared_ptr<AudioClip> bowFireClip;
	std::shared_ptr<AudioClip> arrowFlying;
	std::shared_ptr<Sound> arrowPassbySound;

	uint64_t attackHitConnection = 0;
	DirectX::XMFLOAT3 knockbackVelocity{ 0.0f, 0.0f, 0.0f };
	float knockbackStunTimer = 0.0f;
	float movementDistanceAccumulator = 0.0f;
	float lastFootstepAnimTime = -1.0f;
	float randomSoundTimer = 0.0f;

	const float hitBloodRadiusMin = 0.5f;
	const float hitBloodRadiusMax = 1.5f;
	const float deathBloodRadiusMin = 1.5f;
	const float deathBloodRadiusMax = 2.5f;

	const float ogreBloodRediousMult = 1.5f;

	bool isAttacking = false;
	bool hasAttacked = false;

	std::shared_ptr<Enemy> enemy = nullptr;

	// attack
	float attackElapsed = 0.0f;
	bool hitboxActivated = false;
	// enemy type
	uint32_t wave = 0;

	// arrow for Skeleton

	ExportPropertyCategory("Arrow");
	ExportProperty(float, arrowDamage,50.0f);
	ExportProperty(XMFLOAT3, arrowOffset, { 0.0f, 1.4f, 0.0f });
	ExportProperty(float, arrowGravity, 16.0f);
	ExportProperty(float, arrowSpeed, 25.0f);
	ExportProperty(float, cooldown, 4.0f);
	ExportProperty(float, minShootRadius, 6.0f);
	ExportProperty(float, maxShootRadius, 14.0f);
	ExportProperty(float, fleeSpeedMultiplier, 0.8f);

	float shootTimer = 0.0f;
	Entity* arrow = nullptr;
	std::shared_ptr<Model> arrowModel;
	std::shared_ptr<Texture2D> flameTexture;
	std::shared_ptr<Texture2D> smokeTexture;

	// Fireball
	ExportPropertyCategory("Fireball");
	ExportProperty(XMFLOAT2, XandY, { 0,0 });
	ExportProperty(float, fireballRadius, 0.3f);
	ExportProperty(float, fireballDamage, 50.0f);
	ExportProperty(XMFLOAT3, fireballOffset, { 0.0f, 1.4f, 0.0f });

	ExportProperty(float, fireballGravity, 16.0f);
	ExportProperty(float, fireballVerticalGravity, 0.1f);
	ExportProperty(float, fireballHorizontalGravity, 0.2f);

	ExportProperty(float, fireballSpeed, 25.0f);
	ExportProperty(float, fireballVerticalSpeed, 21.5f);
	ExportProperty(float, fireballHorizontalSpeed, 21.5f);

	ExportProperty(float, fireballCooldown, 2.0f);
	ExportProperty(int, fireballBurstCount, 3);
	ExportProperty(float, fireballBurstInterval, 0.25f);

	float attackCooldownTimer = 0.0f;
	int fireballBurstRemaining = 0;
	float fireballBurstTimer = 0.0f;

	Scene* thisScene = nullptr;

	void OnStart(const AppContext &context) override;
    void Update(const AppContext &context, float deltaTime) override;
	void OnDestroy(const AppContext &context) override;

	void OnAttackHit(const AttackHitEvent& event);

	void Move(float deltaTime);

	void OnDeath(const AppContext &context);

	bool CanAttack(float distanceSq);

	void StartAttack(float deltaTime);
	void EndAttack();
	void UpdateAttack(float deltaTime);

	void Attack();
	void MeleeAttack(float damage, float duration, XMFLOAT3 hitPosition, XMFLOAT3 hitExtends);

	void FireProjectile();

	void UpdateBark(float deltaTime);
	void UpdateKnockback(float deltaTime);

public:
    float emissiveTimer = 0.0f;
    float emissiveDuration = 0.1f;

    EnemyControllerComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~EnemyControllerComponent() = default;

	DirectX::XMFLOAT3 GetPosition() const;
	void UpdateAI(float deltaTime, size_t myGridIndex, const SpatialGrid &grid);

	void ApplyKnockback(const DirectX::XMFLOAT3& direction, float force);
	void ResetKnockback();

	float GetStopDistance() const { return this->stopDistance; }
	void SetStopDistance(float distance) { this->stopDistance = distance; }

	std::shared_ptr<Sound> GetHitSound() const { return this->hitSound; }
	void SetHitSound(std::shared_ptr<Sound> sound) { this->hitSound = sound; }

	std::shared_ptr<Sound> GetDeathSound() const { return this->deathSound; }
	void SetDeathSound(std::shared_ptr<Sound> sound) { this->deathSound = sound; }

	std::shared_ptr<Sound> GetAttackHitSound() const { return this->attackHitSound; }
	void SetAttackHitSound(std::shared_ptr<Sound> sound) { this->attackHitSound = sound; }

	float GetFootstepInterval() const { return this->footstepIntervalMeters; }
	void SetFootstepInterval(float meters) { this->footstepIntervalMeters = meters; }

	void SetGibs(GibsComponent *gibs) { this->gibs = gibs; }

	void SetEnemy(std::shared_ptr<Enemy> enemy) { this->enemy = enemy; }

	bool IsKnockedBack() const { return (this->knockbackVelocity.x * this->knockbackVelocity.x + this->knockbackVelocity.z * this->knockbackVelocity.z) > 0.001f; }

	void SetWave(uint32_t wave) { this->wave = wave; }
	uint32_t GetWave() const { return this->wave; }
	
	void ShootAnArrow();


	void ShootAnFireball(float gravity, float fireSpeed, XMFLOAT3 offset, bool wall = false);
	void ShootVerticalRowOfFireball();
	void ShootHorizontalRowOfFireball();
	void ShootCircleOfFireball();
	void ShootRainOfFireball();
	void ShootWallOfFireball();
	void WizardFireballAttack();
};
