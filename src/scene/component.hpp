#pragma once

#include "core/app_context.hpp"
#include "rendering/render_view.hpp"

#include "reflection/TypeInfo.h"
#include "reflection/MemberVars.h"
#include "reflection/reflection_utils.hpp"

#include <DirectXCollision.h>

// Call these in the constructor to indicate that a component never wants Update()/Render()
// Not strictly necessary, just removes a virtual function call. 
#define UsesFixedUpdate() this->wantsFixedUpdate = true
#define UsesUpdate() this->wantsUpdate = true
#define UsesLateUpdate() this->wantsLateUpdate = true
#define UsesRender() this->wantsRender = true

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
    bool wantsFixedUpdate = false;
    bool wantsUpdate = false;
    bool wantsLateUpdate = false;
    bool wantsRender = false;

    // A component may only be created via Entity::AddComponent()
    Component(Entity *owner, bool isActive = true) : owner(owner), isActive(isActive) {}

private:
    // These should only ever be called through the main frame loop
    virtual void OnStart(const AppContext &context) {}
    virtual void FixedUpdate(const AppContext &context, float deltaTime) {}
    virtual void Update(const AppContext &context, float deltaTime) {}
    virtual void LateUpdate(const AppContext &context, float deltaTime) {}
    virtual void Render(const RenderView &view, RenderQueue &queue) {}
    virtual void OnDestroy(const AppContext &context) {}

public:
    virtual ~Component() = default;

    virtual bool GetWorldBounds(DirectX::BoundingBox &outBounds) { return false; }

    Entity *GetOwner() const { return this->owner; }

    bool IsActive() const { return this->isActive; }
    void SetActive(bool isActive) { this->isActive = isActive; }

    bool IsPendingDestroy() const { return this->isPendingDestroy; }

    bool WantsFixedUpdate() const { return this->wantsFixedUpdate; }
    bool WantsUpdate() const { return this->wantsUpdate; }
    bool WantsLateUpdate() const { return this->wantsLateUpdate; }
    bool WantsRender() const { return this->wantsRender; }

    virtual TypeId GetTypeId() const = 0;
};