#include "area_component.hpp"
#include "collision/collision_component.hpp"

void AreaComponent::OnCollision(const CollisionEvent &e)
{
    if (!e.selfCollider || e.selfCollider->GetOwner() != this->GetOwner())
        return;

    if (this->enabled)
        this->onAreaEntered.Emit(e.otherCollider);
}

void AreaComponent::OnStart(const AppContext &context)
{
    this->collisionEventId = EventBus::Subscribe<CollisionEvent>(
        this, &AreaComponent::OnCollision
    );
}

void AreaComponent::OnDestroy(const AppContext &context)
{
    if (this->collisionEventId != 0) {
        EventBus::Unsubscribe<CollisionEvent>(this->collisionEventId);
        this->collisionEventId = 0;
    }
}