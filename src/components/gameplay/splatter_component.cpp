#include "components/gameplay/splatter_component.hpp"
#include "collision/collision_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "core/rng.hpp"
#include "core/logging.hpp"

void SplatterComponent::OnStart(const AppContext& context)
{
    this->hitEventId = EventBus::Subscribe<CollisionEvent>(
        this, &SplatterComponent::OnCollisionEvent
    );
}

void SplatterComponent::Update(const AppContext& context, float deltaTime)
{
}

void SplatterComponent::OnDestroy(const AppContext& context)
{
    if (this->hitEventId != 0) {
        EventBus::Unsubscribe<CollisionEvent>(this->hitEventId);
        this->hitEventId = 0;
    }
}

void SplatterComponent::OnCollisionEvent(const CollisionEvent& e)
{
    if (!e.selfCollider || e.selfCollider->GetOwner() != this->GetOwner())
        return;

    if (!e.otherCollider || !e.otherCollider->GetOwner())
        return;

    Entity* otherOwner = e.otherCollider->GetOwner();
    if (!otherOwner->IsActive())
        return;

    PaintableComponent* paintable = otherOwner->GetComponent<PaintableComponent>();
    if (!paintable)
        return;

    TransformComponent* tr = this->GetOwner()->GetComponent<TransformComponent>();
    if (!tr)
        return;

    const DirectX::XMFLOAT3 now = tr->GetWorldPosition();

    if (this->hasPaintedOnce)
    {
        const float dx = now.x - this->lastPaintPos.x;
        const float dz = now.z - this->lastPaintPos.z;
        const float distSq = dx * dx + dz * dz;

        if (distSq < this->lastPaintDist * this->lastPaintDist)
            return;
    }

    paintable->PaintAtPosition(now, RNG::Float(0.5,2));

    this->lastPaintPos = now;
    this->hasPaintedOnce = true;
}