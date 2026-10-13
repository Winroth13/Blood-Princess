#pragma once
#include "components/core/transform_component.hpp"
#include "scene/component.hpp"
#include "scene/entity.hpp"
#include "events/event_bus.hpp"

class PaintableComponent;

class SplatterComponent :
    public Component
{
    RegisterComponent(SplatterComponent);
    
    float lastPaintDist = 1.0f;
    DirectX::XMFLOAT3 lastPaintPos = { 0, 0, 0 };
    bool hasPaintedOnce = false;

    ConnectionId hitEventId = 0;
private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;

    void OnCollisionEvent(const CollisionEvent& e);
public:
    SplatterComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
    void PaintFloorAtPosition(const DirectX::XMFLOAT3& position, float radius);

    void SetPaintSpacing(float distance) { this->lastPaintDist = distance; }
    void Reset();
};

