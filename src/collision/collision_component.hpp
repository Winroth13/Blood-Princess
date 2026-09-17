#pragma once
#include "scene/component.hpp"
#include "collision/collision_layer.hpp"
#include "collision/Rect_2d.hpp"
class CollisionComponent : public Component
{
protected:
    CollisionLayer layer = CollisionLayer::ENVIRONMENT;
    bool isTrigger = false;
    bool isStatic = false;
    DirectX::XMFLOAT3 centerOffset = { 0.0f, 0.0f, 0.0f };
    
public:
    CollisionComponent(Entity *owner, bool isActive) : Component(owner, isActive) { NoUpdate(); NoRender(); }
    virtual ~CollisionComponent() = default;
    void OnStart(const AppContext& context) override {};
    void Update(const AppContext &context, float deltaTime) override {};
    void Render(const RenderView &view, RenderQueue &queue) override {};
    void OnDestroy(const AppContext &context) override {};

    //virtual Rect2D GetBoundingRect2D() { return Rect2D(1,1,1,1); }
    virtual bool Intersects(CollisionComponent* other) { return false; };
    virtual void OnTriggerEnter(CollisionComponent* other) {}
    virtual void OnTriggerExit(CollisionComponent* other) {}
    virtual void OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal) {}


    CollisionLayer GetLayer() const { return this->layer; }
    void SetLayer(CollisionLayer layer) { this->layer = layer; }

    bool IsTrigger() const { return this->isTrigger; }
    void SetTrigger(bool trigger) { this->isTrigger = trigger; }

    bool IsStatic() const { return this->isStatic; }
    void SetStatic(bool isStatic) { this->isStatic = isStatic; }

    DirectX::XMFLOAT3 GetCenterOffset() const { return this->centerOffset; }
    void SetCenterOffset(const DirectX::XMFLOAT3& offset) { this->centerOffset = offset; }
};
