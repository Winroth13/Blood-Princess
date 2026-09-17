#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"
#include "collision/collision_component.hpp"

class SphereCollisionComponent;
class CapsuleCollisionComponent;

class BoxCollisionComponent : public CollisionComponent
{
	RegisterComponent(BoxCollisionComponent);
	ExportProperty(XMFLOAT3, extents, { 0.5f, 0.5f, 0.5f });
	DirectX::BoundingOrientedBox worldOBB{};
public:


	BoxCollisionComponent(Entity* owner, bool isActive) : CollisionComponent(owner, isActive){};

	//Rect2D GetBoundingRect2D() override { return rect2D; };
	bool Intersects(CollisionComponent* other) override { return true; };

	BoundingOrientedBox GetWorldOBB() const { return this->worldOBB; }
	void UpdateWorldOBB();

	XMFLOAT3 GetExtents() const { return this->extents; };
	void SetExtents(const XMFLOAT3& extents) { this->extents = extents; }
};