#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"
#include "collision/collision_component.hpp"
#include "scene/entity.hpp"
//class SphereCollisionComponent;
//class CapsuleCollisionComponent;

class BoxCollisionComponent : public CollisionComponent
{
	RegisterComponent(BoxCollisionComponent);
	ExportProperty(XMFLOAT3, extents, { 0.5f, 0.5f, 0.5f });
	
	DirectX::BoundingOrientedBox worldOBB{};
public:
	BoxCollisionComponent(Entity* owner, bool isActive);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;

	bool Intersects(CollisionComponent* other) override;
	void OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal) override;

	BoundingOrientedBox GetWorldOBB() const { return this->worldOBB; }
	void UpdateWorldOBB();

	XMFLOAT3 GetExtents() const { return this->extents; };
	void SetExtents(const XMFLOAT3& extents) { this->extents = extents; }
};