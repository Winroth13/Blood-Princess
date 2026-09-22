#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"
#include "collision/collision_component.hpp"

class CapsuleCollisionComponent : public CollisionComponent
{
	RegisterComponent(CapsuleCollisionComponent);
	ExportProperty(float, radius, 0.5f);
	ExportProperty(float, height, 2.0f);

public:
	CapsuleCollisionComponent(Entity* owner, bool isActive) : CollisionComponent(owner, isActive) {};

	Rect2D GetBoundingRect2D() override;
	bool Intersects(CollisionComponent* other) override;

	float GetRadius() const { return this->radius; };
	void SetRadius(const float radius) { this->radius = radius; }

};