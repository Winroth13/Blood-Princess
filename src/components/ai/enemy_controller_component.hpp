#pragma once

#include "scene/component.hpp"
#include "ai/ai_manager.hpp"
#include <DirectXMath.h>

class TransformComponent;
class SpatialGrid;
struct AttackHitEvent;

class EnemyControllerComponent : public Component
{
	RegisterComponent(EnemyControllerComponent);
	ExportProperty(std::string, groupName, "Enemy");
	ExportProperty(float, moveSpeed, 3.5f);
	ExportProperty(float, avoidanceRadius, 1.5f);
	ExportProperty(float, stopDistance, 2.0f);
	ExportProperty(float, knockbackMultiplier, 4.0f);
	ExportProperty(float, knockbackDecay, 12.0f);
	ExportProperty(float, knockbackStunDuration, 0.2f);

private:
	TransformComponent *transform = nullptr;
	AIManager* aiManager = nullptr;
	uint64_t attackHitConnection = 0;
	DirectX::XMFLOAT3 knockbackVelocity{ 0.0f, 0.0f, 0.0f };
	float knockbackStunTimer = 0.0f;

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

	bool IsKnockedBack() const { return (this->knockbackVelocity.x * this->knockbackVelocity.x + this->knockbackVelocity.z * this->knockbackVelocity.z) > 0.001f; }
};
