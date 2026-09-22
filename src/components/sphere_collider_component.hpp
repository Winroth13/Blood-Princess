#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"
#include "collision/collision_component.hpp"

class SphereCollisionComponent : public CollisionComponent 
{
	RegisterComponent(SphereCollisionComponent);
	ExportProperty(float, radius, 1.0f);
	BoundingSphere worldSphere{};

public:
	SphereCollisionComponent(Entity* owner, bool isActive) : CollisionComponent(owner, isActive){};

	Rect2D GetBoundingRect2D() override;
	bool Intersects(CollisionComponent* other) override;

	BoundingSphere GetWorldSphere() const { return this->worldSphere; }
	void UpdateWorldOBB();

	float GetRadius() const { return this->radius; };
	void SetRadius(const float radius) { this->radius = radius; }

};