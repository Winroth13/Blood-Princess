#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"
#include "collision/collision_component.hpp"
#include "scene/entity.hpp"

class SphereCollisionComponent : public CollisionComponent 
{
	RegisterComponent(SphereCollisionComponent);
	ExportProperty(float, radius, 1.0f);
	BoundingSphere worldSphere{};

public:
	SphereCollisionComponent(Entity *owner, bool isActive);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext &context, float deltaTime) override;
	void LateUpdate(const AppContext &context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;

	bool Intersects(CollisionComponent* other) override;
	void OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal) override;

	BoundingSphere GetWorldSphere() const { return this->worldSphere; }
	bool GetWorldBounds(BoundingBox &outerBox) override;
	void UpdateWorldSphere();

	float GetRadius() const { return this->radius; }
	void SetRadius(float radius) { this->radius = radius; }

};