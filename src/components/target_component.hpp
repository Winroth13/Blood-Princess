#pragma once
#include "scene/component.hpp"
#include "scene/entity.hpp"
#include <DirectXMath.h>
#include "components/transform_component.hpp"
class TargetComponent : public Component
{
    RegisterComponent(TargetComponent);

    Entity* target = nullptr;



private:

public:
    TargetComponent(Entity* owner, bool isActive)
        : Component(owner, isActive) {}
    void SetTarget(Entity* target) { this->target = target; }
    DirectX::XMFLOAT3 GetTargetPosition() { return this->GetOwner()->GetComponent<TransformComponent>()->GetWorldPosition(); }
};