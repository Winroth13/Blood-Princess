#pragma once
#include "scene/component.hpp"
#include "events/event_bus.hpp"

class AreaComponent : public Component
{
	RegisterComponent(AreaComponent);

	ExportProperty(bool, enabled, true);

	Signal<CollisionComponent*> onAreaEntered;

	ConnectionId collisionEventId = 0;

public:
	AreaComponent(Entity *owner, bool isActive) : Component(owner, isActive) {}
	~AreaComponent() = default;

	Signal<CollisionComponent *> &GetOnAreaEntered() { return this->onAreaEntered; }

	void OnCollision(const CollisionEvent &e);

	bool IsEnabled() const { return this->enabled; }
	void SetEnabled(bool value) { this->enabled = value; }

private:
	void OnStart(const AppContext &context) override;
	void OnDestroy(const AppContext &context) override;
};