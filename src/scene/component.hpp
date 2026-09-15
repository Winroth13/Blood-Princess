#pragma once

#include "core/app_context.hpp"
#include "rendering/render_view.hpp"

#include "reflection/TypeInfo.h"
#include "reflection/MemberVars.h"
#include "reflection/reflection_utils.hpp"

#include <DirectXCollision.h>

#define NoUpdate() this->wantsUpdate = false
#define NoRender() this->wantsRender = false

class Entity;

class Component {
    Entity *owner = nullptr;

    bool isActive = true;

    bool isPendingDestroy = false;
    friend class Scene;

protected:
    bool wantsUpdate = true;
    bool wantsRender = true;

public:
    Component(Entity *owner, bool isActive = true) : owner(owner), isActive(isActive) {}
    virtual ~Component() = default;

    Component(const Component &other) = delete;
    Component &operator=(const Component &other) = delete;

    virtual void OnStart(const AppContext &context) = 0;
    virtual void Update(const AppContext &context, float deltaTime) = 0;
    virtual void Render(const RenderView &view, RenderQueue &queue) = 0;
    virtual void OnDestroy(const AppContext &context) = 0;

    virtual bool GetWorldBounds(DirectX::BoundingBox &outBounds) { return false; }

    Entity *GetOwner() const { return this->owner; }

    bool IsActive() const { return this->isActive; }
    void SetActive(bool isActive) { this->isActive = isActive; }

    bool IsPendingDestroy() const { return this->isPendingDestroy; }

    bool WantsUpdate() const { return this->wantsUpdate; }
    bool WantsRender() const { return this->wantsRender; }

    virtual TypeId GetTypeId() const = 0;
};