#pragma once

#include "core/app_context.hpp"

class Entity;

class Component {
    Entity *owner = nullptr;

    bool isActive = true;

    bool isPendingDesroy = false;
    friend class Entity;

public:
    Component(Entity *owner, bool isActive = true) : owner(owner), isActive(isActive) {}
    ~Component() = default;

    Component(const Component &other) = delete;
    Component &operator=(const Component &other) = delete;

    virtual void OnStart(const AppContext &context) = 0;
    virtual void Update(const AppContext &context, float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void OnDestroy(const AppContext &context) = 0;

    Entity *GetOwner() const { return this->owner; }

    bool IsActive() const { return this->isActive; }
    void SetActive(bool isActive) { this->isActive = isActive; }

    bool IsPendingDestroy() const { return this->isPendingDesroy; }
};