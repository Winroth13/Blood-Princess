#pragma once

#include "core/app_context.hpp"
#include "rendering/render_view.hpp"

#include "reflection/TypeInfo.h"
#include "reflection/MemberVars.h"
#include "reflection/reflection_utils.hpp"

#include <DirectXCollision.h>

// Call these in the constructor to indicate that a component never wants Update()/Render()
// Not strictly necessary, just removes a virtual function call. 
#define NoUpdate() this->wantsUpdate = false
#define NoLateUpdate() this->wantsLateUpdate = false
#define NoRender() this->wantsRender = false

class Entity;

class Component {
    friend class Scene;
    friend class SceneCuller;
    friend class Entity;

    Entity *owner = nullptr;

    // A component that is inactive will not receive Update() or Render()
    bool isActive = true;
    bool isPendingDestroy = false;

protected:
    bool wantsUpdate = true;
    bool wantsLateUpdate = true;
    bool wantsRender = true;

    // A component may only be created via Entity::AddComponent()
    Component(Entity *owner, bool isActive = true) : owner(owner), isActive(isActive) {}

private:
    // These should only ever be called through the main frame loop
    virtual void OnStart(const AppContext &context) = 0;
    virtual void Update(const AppContext &context, float deltaTime) = 0;
    virtual void LateUpdate(const AppContext &context, float deltaTime) = 0;
    virtual void Render(const RenderView &view, RenderQueue &queue) = 0;
    virtual void OnDestroy(const AppContext &context) = 0;

public:
    virtual ~Component() = default;

    virtual bool GetWorldBounds(DirectX::BoundingBox &outBounds) { return false; }

    Entity *GetOwner() const { return this->owner; }

    bool IsActive() const { return this->isActive; }
    void SetActive(bool isActive) { this->isActive = isActive; }

    bool IsPendingDestroy() const { return this->isPendingDestroy; }

    bool WantsUpdate() const { return this->wantsUpdate; }
    bool WantsRender() const { return this->wantsRender; }

    virtual TypeId GetTypeId() const = 0;
};