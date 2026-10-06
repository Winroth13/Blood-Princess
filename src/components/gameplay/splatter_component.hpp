#pragma once
#include "components/core/transform_component.hpp"
#include "scene/component.hpp"
#include "scene/entity.hpp"
#include "events/event_bus.hpp"


class SplatterComponent :
    public Component
{
    RegisterComponent(SplatterComponent);

    ConnectionId hitEventId;
private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;
public:
    SplatterComponent(Entity* owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
};

