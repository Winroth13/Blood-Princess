#pragma once

#include "scene/component.hpp"
#include "ai/ai_manager.hpp"
#include <DirectXMath.h>

class TransformComponent;
class AnimatorComponent;
class SpatialGrid;
class Sound;
struct AttackHitEvent;

class EnemyControllerComponent : public Component
{
	RegisterComponent(EnemyControllerComponent);
	ExportProperty(std::string, groupName, "Enemy");
	ExportProperty(float, moveSpeed, 3.5f);
	ExportProperty(float, avoidanceRadius, 1.5f);
	ExportProperty(float, stopDistance, 1.0f);
	ExportProperty(float, knockbackMultiplier, 4.0f);
	ExportProperty(float, knockbackDecay, 12.0f);
	ExportProperty(float, knockbackStunDuration, 0.2f);
	ExportProperty(std::shared_ptr<Sound>, hitSound, nullptr);
	ExportProperty(std::shared_ptr<Sound>, deathSound, nullptr);
	ExportProperty(std::shared_ptr<Sound>, footstepSound, nullptr);
	ExportProperty(float, footstepIntervalMeters, 2.5f);
	ExportProperty(std::shared_ptr<Sound>, randomSound, nullptr);
	ExportProperty(float, randomSoundMinInterval, 5.0f);
	ExportProperty(float, randomSoundMaxInterval, 50.0f);

private:
	TransformComponent *transform = nullptr;
	AnimatorComponent *animator = nullptr;
	AIManager* aiManager = nullptr;
	uint64_t attackHitConnection = 0;
	DirectX::XMFLOAT3 knockbackVelocity{ 0.0f, 0.0f, 0.0f };
	float knockbackStunTimer = 0.0f;
	float movementDistanceAccumulator = 0.0f;
	float randomSoundTimer = 0.0f;
	bool isAttacking = false;

	void OnStart(const AppContext &context) override;
	void OnDestroy(const AppContext &context) override;
	void OnAttackHit(const AttackHitEvent& event);

public:
	EnemyControllerComponent(Entity* owner, bool isActive) : Component(owner, isActive) {};
	~EnemyControllerComponent() = default;

	DirectX::XMFLOAT3 GetPosition() const;
	void UpdateAI(float deltaTime, size_t myGridIndex, const SpatialGrid &grid);

	void ApplyKnockback(const DirectX::XMFLOAT3& direction, float force);
	void ResetKnockback();

	float GetMoveSpeed() const { return this->moveSpeed; }
	void SetMoveSpeed(float speed) { this->moveSpeed = speed; }

	float GetAvoidanceRadius() const { return this->avoidanceRadius; }
	void SetAvoidanceRadius(float radius) { this->avoidanceRadius = radius; }

	float GetStopDistance() const { return this->stopDistance; }
	void SetStopDistance(float distance) { this->stopDistance = distance; }

	float GetKnockbackMultiplier() const { return this->knockbackMultiplier; }
	void SetKnockbackMultiplier(float multiplier) { this->knockbackMultiplier = multiplier; }

	float GetKnockbackDecay() const { return this->knockbackDecay; }
	void SetKnockbackDecay(float decay) { this->knockbackDecay = decay; }

	float GetKnockbackStunDuration() const { return this->knockbackStunDuration; }
	void SetKnockbackStunDuration(float duration) { this->knockbackStunDuration = duration; }

	std::shared_ptr<Sound> GetHitSound() const { return this->hitSound; }
	void SetHitSound(std::shared_ptr<Sound> sound) { this->hitSound = sound; }

	std::shared_ptr<Sound> GetDeathSound() const { return this->deathSound; }
	void SetDeathSound(std::shared_ptr<Sound> sound) { this->deathSound = sound; }

	std::shared_ptr<Sound> GetFootstepSound() const { return this->footstepSound; }
	void SetFootstepSound(std::shared_ptr<Sound> sound) { this->footstepSound = sound; }

	float GetFootstepInterval() const { return this->footstepIntervalMeters; }
	void SetFootstepInterval(float meters) { this->footstepIntervalMeters = meters; }

	std::shared_ptr<Sound> GetRandomSound() const { return this->randomSound; }
	void SetRandomSound(std::shared_ptr<Sound> sound) { this->randomSound = sound; }

	bool IsKnockedBack() const { return (this->knockbackVelocity.x * this->knockbackVelocity.x + this->knockbackVelocity.z * this->knockbackVelocity.z) > 0.001f; }
};
