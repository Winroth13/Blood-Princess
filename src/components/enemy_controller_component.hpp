#pragma once

#include "scene/component.hpp"
#include "ai/ai_manager.hpp"
#include <DirectXMath.h>

class TransformComponent;
class SpatialGrid;

class EnemyControllerComponent : public Component
{
	RegisterComponent(EnemyControllerComponent);
	ExportProperty(std::string, groupName, "Enemy");
	ExportProperty(float, moveSpeed, 3.5f);
	ExportProperty(float, avoidanceRadius, 1.5f);
	ExportProperty(float, aggroRadius, 20.0f);
	ExportProperty(float, stopDistance, 2.0f);

private:
	TransformComponent *transform = nullptr;
	AIManager* aiManager = nullptr;


	void OnStart(const AppContext &context) override;
	void OnDestroy(const AppContext &context) override;

public:
	EnemyControllerComponent(Entity* owner, bool isActive) : Component(owner, isActive) { NoUpdate(), NoLateUpdate(), NoRender(); };
	~EnemyControllerComponent() = default;

	DirectX::XMFLOAT3 GetPosition() const;
	void UpdateAI(float deltaTime, size_t myGridIndex, const SpatialGrid &grid);

	float GetMoveSpeed() const { return this->moveSpeed; }
	void SetMoveSpeed(float speed) { this->moveSpeed = speed; }

	float GetAvoidanceRadius() const { return this->avoidanceRadius; }
	void SetAvoidanceRadius(float radius) { this->avoidanceRadius = radius; }
};
