#include "box_collision_component.hpp"
#include "components/transform_component.hpp"
#include "collision/collision_manager.hpp"
#include "core/logging.hpp"
BoxCollisionComponent::BoxCollisionComponent(Entity* owner, bool isActive) 
    : CollisionComponent(owner, isActive)
{
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

    return false;
}

void BoxCollisionComponent::OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal)
{
    LogInfo("Other: (%f, %f, %f)", normal.x, normal.y, normal.z);
    LogInfo("Hit");

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
