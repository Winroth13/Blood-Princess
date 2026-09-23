#include "box_collision_component.hpp"
#include "components/transform_component.hpp"
#include "collision/collision_manager.hpp"
#include "core/logging.hpp"
#include "components/model_component.hpp"
#include "DirectXMath.h"
#include "components/sphere_collision_component.hpp"
BoxCollisionComponent::BoxCollisionComponent(Entity* owner, bool isActive) 
    : CollisionComponent(owner, isActive)
{
    NoLateUpdate();
    NoRender();
    this->SetShape(CollisionShape::BOX);
}

void BoxCollisionComponent::OnStart(const AppContext& context)
{
    this->UpdateWorldOBB();

    if (context.collisionManager) {
        context.collisionManager->RegisterCollider(this);
    }
}

void BoxCollisionComponent::Update(const AppContext& context, float deltaTime)
{
    UpdateWorldOBB();
}

void BoxCollisionComponent::OnDestroy(const AppContext& context)
{
    if (context.collisionManager) {
        context.collisionManager->UnregisterCollider(this);
    }
}

bool BoxCollisionComponent::Intersects(CollisionComponent* other)
{
    if (!other || !other->IsActive()) return false;

    if (other->GetShape() == CollisionShape::BOX)
    {
        auto otherBox = static_cast<BoxCollisionComponent*>(other);
        return this->worldOBB.Intersects(otherBox->GetWorldOBB());
    }
    if (other->GetShape() == CollisionShape::SPHERE)
    {
        auto otherSphere = static_cast<SphereCollisionComponent*>(other);
        return this->worldOBB.Intersects(otherSphere->GetWorldSphere());
    }

    return false;
}

void BoxCollisionComponent::OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal)
{
    LogInfo("Other: (%f, %f, %f)", normal.x, normal.y, normal.z);
    LogInfo("Hit");

}


bool BoxCollisionComponent::GetWorldBounds(BoundingBox& outerBox)
{
    DirectX::XMFLOAT3 corners[DirectX::BoundingOrientedBox::CORNER_COUNT];

    worldOBB.GetCorners(corners);

    DirectX::BoundingBox::CreateFromPoints(outerBox, DirectX::BoundingOrientedBox::CORNER_COUNT, corners, sizeof(DirectX::XMFLOAT3));

    return true;
}

void BoxCollisionComponent::UpdateWorldOBB()
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

    DirectX::BoundingOrientedBox localBox;
    localBox.Center = this->centerOffset;
    localBox.Extents = this->extents;
    localBox.Orientation = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    localBox.Transform(this->worldOBB, transform->GetWorldMatrix());
}

void BoxCollisionComponent::SetExtents(const XMFLOAT3& extents)
{
    this->extents = extents;
    this->worldOBB.Extents = this->extents;
}

void BoxCollisionComponent::UpdateExtents(float width, float height, float depth)
{
    this->extents = DirectX::XMFLOAT3(width / 2.0f, height / 2.0f, depth / 2.0f);

    UpdateWorldOBB();
}
