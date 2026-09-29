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
    void OnStart(const AppContext& context) override {}
    void Update(const AppContext& context, float deltaTime) override {}
    void LateUpdate(const AppContext& context, float deltaTime) override {}
    void Render(const RenderView& view, RenderQueue& queue) override {}
    void OnDestroy(const AppContext& context) override {}

public:
    TargetComponent(Entity* owner, bool isActive)
        : Component(owner, isActive)
    {
        NoUpdate();
        NoLateUpdate();
        NoRender();
    }
    void SetTarget(Entity* target) { this->target = target; }
    DirectX::XMFLOAT3 GetTargetPosition() { return this->GetOwner()->GetComponent<TransformComponent>()->GetWorldPosition(); }
};