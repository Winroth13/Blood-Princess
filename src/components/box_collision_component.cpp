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

    auto otherBox = static_cast<BoxCollisionComponent*>(other);
    return this->worldOBB.Intersects(otherBox->GetWorldOBB());

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
    DirectX::XMFLOAT3 worldPos = transform->GetWorldPosition();
    DirectX::XMFLOAT3 worldScale = transform->GetWorldScale();
    DirectX::XMFLOAT4 worldRot = transform->GetWorldRotationQuaternion();
    
    this->worldOBB.Center = DirectX::XMFLOAT3(
        worldPos.x + this->centerOffset.x,
        worldPos.y + this->centerOffset.y,
        worldPos.z + this->centerOffset.z
    );
    
    this->worldOBB.Extents = DirectX::XMFLOAT3(
        this->extents.x * worldScale.x,
        this->extents.y * worldScale.y,
        this->extents.z * worldScale.z
    );
    
    this->worldOBB.Orientation = worldRot;
}
