#include "sphere_collision_component.hpp"
#include "components/transform_component.hpp"
#include "collision/collision_manager.hpp"
#include "components/box_collision_component.hpp"
#include "core/logging.hpp"

SphereCollisionComponent::SphereCollisionComponent(Entity* owner, bool isActive)
	: CollisionComponent(owner, isActive)
{
	UsesUpdate();
	this->shape = CollisionShape::SPHERE;
}

void SphereCollisionComponent::OnStart(const AppContext& context)
{
	this->UpdateWorldSphere();
	if (context.collisionManager) {
		context.collisionManager->RegisterCollider(this);
	}
}

void SphereCollisionComponent::Update(const AppContext& context, float deltaTime)
{
	this->UpdateWorldSphere();
}

void SphereCollisionComponent::OnDestroy(const AppContext& context)
{
	if (context.collisionManager) {
		context.collisionManager->UnregisterCollider(this);
	}
}

bool SphereCollisionComponent::Intersects(CollisionComponent* other)
{
	if (!other || !other->IsActive()) return false;

	if (other->GetShape() == CollisionShape::BOX) {
		auto otherBox = static_cast<BoxCollisionComponent*>(other);
		return this->worldSphere.Intersects(otherBox->GetWorldOBB());
	}
	if (other->GetShape() == CollisionShape::SPHERE)
	{
		auto otherSphere = static_cast<SphereCollisionComponent*>(other);
		return this->worldSphere.Intersects(otherSphere->GetWorldSphere());
	}

	return false;
}

void SphereCollisionComponent::OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal)
{
	EventBus::Publish(CollisionEvent{ this, other, normal });
}

bool SphereCollisionComponent::GetWorldBounds(BoundingBox& outerBox)
{
	DirectX::XMFLOAT3 min{
		worldSphere.Center.x - this->radius,
		worldSphere.Center.y - this->radius,
		worldSphere.Center.z - this->radius
	};
	DirectX::XMFLOAT3 max{
		worldSphere.Center.x + this->radius,
		worldSphere.Center.y + this->radius,
		worldSphere.Center.z + this->radius
	};
	BoundingBox worldBox = BoundingBox(min, max);
	outerBox = worldBox;
	return true;
}

bool SphereCollisionComponent::GetBounds2D(Rect2D& outRect) const
{
	outRect = Rect2D(
		this->worldSphere.Center.x - this->worldSphere.Radius,
		this->worldSphere.Center.x + this->worldSphere.Radius,
		this->worldSphere.Center.z - this->worldSphere.Radius,
		this->worldSphere.Center.z + this->worldSphere.Radius
	);
	return true;
}

void SphereCollisionComponent::UpdateWorldSphere()
{
	Entity* ownerEntity = this->GetOwner();
	if (!ownerEntity)
	{
		return;
	}
	TransformComponent* transform = ownerEntity->GetComponent<TransformComponent>();
	if (!transform)
	{
		return;
	}
	BoundingSphere localSphere{};
	localSphere.Center = this->centerOffset;
	localSphere.Radius = this->radius;
	localSphere.Transform(this->worldSphere, transform->GetWorldMatrix());
}
