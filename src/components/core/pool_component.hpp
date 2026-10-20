#pragma once
#include "scene/component.hpp"

#include <functional>
#include <stack>

class PoolComponent : public Component
{
    RegisterComponent(PoolComponent);

    ExportProperty(int, size, 128);
    ExportPropertyChanged(size, Resize);

    std::function<Entity *()> createEntityCallback;
    std::stack<Entity *> inactive = {};
    std::stack<Entity *> active = {};

public:
    PoolComponent(Entity *owner, bool isActive) : Component(owner, isActive) { }

    void SetCreateEntityCallback(std::function<Entity *()> callback) { this->createEntityCallback = callback; }

    void SetSize(int size);

    Entity *Grab();
    void Put(Entity* entity);

private:
    void OnStart(const AppContext &context) override;

    void Resize();
};