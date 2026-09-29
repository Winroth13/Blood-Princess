#pragma once

#include "scene/component.hpp"

#include <DirectXMath.h>

class TransformComponent;
class SpatialGrid;

class EnemyControllerComponent : public Component
{
	RegisterComponent(EnemyControllerComponent);

	ExportProperty(float, moveSpeed, 3.5f);
	ExportProperty(float, avoidanceRadius, 1.5f);

private:
	TransformComponent *transform = nullptr;

public:
	EnemyControllerComponent(Entity *owner, bool isActive);
	~EnemyControllerComponent() = default;

private:
	void OnStart(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override {}
	void LateUpdate(const AppContext &context, float deltaTime) override {}
	void Render(const RenderView &view, RenderQueue &queue) override {}
	void OnDestroy(const AppContext &context) override;

public:
	DirectX::XMFLOAT3 GetPosition() const;
	void UpdateAI(float deltaTime, const DirectX::XMFLOAT3 &playerPosition, size_t myGridIndex, const SpatialGrid &grid);

	float GetMoveSpeed() const { return this->moveSpeed; }
	void SetMoveSpeed(float speed) { this->moveSpeed = speed; }

	float GetAvoidanceRadius() const { return this->avoidanceRadius; }
	void SetAvoidanceRadius(float radius) { this->avoidanceRadius = radius; }
};
