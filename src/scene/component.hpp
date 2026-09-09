#pragma once

#include "core/app_context.hpp"
#include "rendering/render_view.hpp"

#include <DirectXCollision.h>

using namespace DirectX;

class Entity;

class Component {
    Entity *owner = nullptr;

    bool isActive = true;

    bool isPendingDestroy = false;
    friend class Scene;

public:
    Component(Entity *owner, bool isActive = true) : owner(owner), isActive(isActive) {}
    ~Component() = default;

    Component(const Component &other) = delete;
    Component &operator=(const Component &other) = delete;

    // OnStart runs in the order that entities are added
    virtual void OnStart(const AppContext &context) = 0;
    virtual void Update(const AppContext &context, float deltaTime) = 0;
    virtual void Render(const RenderView &view, RenderQueue &queue) = 0;
    virtual void OnDestroy(const AppContext &context) = 0;

    virtual bool GetWorldBounds(BoundingBox &outBounds) { return false; }

    Entity *GetOwner() const { return this->owner; }

    bool IsActive() const { return this->isActive; }
    void SetActive(bool isActive) { this->isActive = isActive; }

    bool IsPendingDestroy() const { return this->isPendingDestroy; }
};