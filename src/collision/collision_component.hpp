#pragma once
#include "scene/component.hpp"
#include "collision/collision_layer.hpp"
#include "collision/Rect_2d.hpp"

enum class CollisionShape {
    BOX,
    SPHERE,
    CAPSULE
};

class CollisionComponent : public Component
{
protected:
    CollisionLayer layer = CollisionLayer::ENVIRONMENT;
    CollisionShape shape;
    bool isTrigger = false;
    bool isStatic = false;
    DirectX::XMFLOAT3 centerOffset = { 0.0f, 0.0f, 0.0f };
    
public:
    CollisionComponent(Entity* owner, bool isActive) : Component(owner, isActive) { NoUpdate(); NoRender(); };
    virtual ~CollisionComponent() = default;

    virtual bool Intersects(CollisionComponent* other) = 0;
    virtual void OnCollision(CollisionComponent* other, DirectX::XMFLOAT3 normal) = 0;

    //virtual void OnTriggerEnter(CollisionComponent* other) {}
    //virtual void OnTriggerExit(CollisionComponent* other) {}

    CollisionLayer GetLayer() const { return this->layer; }
    void SetLayer(CollisionLayer layer) { this->layer = layer; }

    bool IsStatic() const { return this->isStatic; }
    void SetStatic(bool isStatic) { this->isStatic = isStatic; }

    bool IsTrigger() const { return this->isTrigger; }
    void SetTrigger(bool trigger) { this->isTrigger = trigger; }


    DirectX::XMFLOAT3 GetCenterOffset() const { return this->centerOffset; }
    void SetCenterOffset(const DirectX::XMFLOAT3& offset) { this->centerOffset = offset; }

    CollisionShape GetShape() const { return this->shape; };
    void SetShape(CollisionShape shape) { this->shape = shape; };
};
