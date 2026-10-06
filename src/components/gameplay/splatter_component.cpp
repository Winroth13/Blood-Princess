#include "components/gameplay/splatter_component.hpp"
#include "collision/collision_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/core/transform_component.hpp"


void SplatterComponent::OnStart(const AppContext& context)
{

    this->hitEventId = EventBus::Subscribe<CollisionEvent>(
        [this](const CollisionEvent& e) {
            if (const auto& paintwall = e.otherCollider->GetOwner()->GetComponent<PaintableComponent>()) {
                paintwall->PaintAtPosition(this->GetOwner()->GetComponent<TransformComponent>()->GetWorldPosition());
            }
        }
    );
}

void SplatterComponent::Update(const AppContext& context, float deltaTime)
{
}

void SplatterComponent::OnDestroy(const AppContext& context)
{
    //EventBus::Unsubscribe(this->hitEventId);
}
