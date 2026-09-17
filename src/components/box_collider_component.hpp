#pragma once
#include "scene/component.hpp"
#include "DirectXMath.h"
#include "d3d11.h"
#include "collision/Rect_2d.hpp"


class SphereColliderComponent;
class CapsuleColliderComponent;

class BoxColliderComponent : public Component
{
	RegisterComponent(BoxColliderComponent);
	ExportProperty(XMFLOAT3, extents, { 0.5f, 0.5f, 0.5f });
	DirectX::BoundingOrientedBox worldOBB{};

public:

	BoxColliderComponent(Entity* owner, bool isActive) : Component(owner, isActive){ NoRender();};

	//Rect2D GetBoundingRect2D() override;
	//bool Intersects() override;

	BoundingOrientedBox GetWorldOBB() const { return this->worldOBB; }
	void UpdateWorldOBB();

	XMFLOAT3 GetExtents() const { return this->extents; };
	void SetExtents(const XMFLOAT3& extents) { this->extents = extents; }




private:

};