#pragma once
#include "scene/component.hpp"
#include "collision/collision_layer.hpp"
#include "collision/Rect_2d.hpp"
#include "events/event_bus.hpp"

enum class CollisionShape {
    NONE,
    BOX,
    SPHERE,
    CAPSULE,
    COUNT
};

class CollisionComponent : public Component
{
protected:
    CollisionLayer layer = CollisionLayer::ENVIRONMENT;
    CollisionShape shape = CollisionShape::NONE;
    bool isTrigger = false;
    bool isStatic = false;
    DirectX::XMFLOAT3 centerOffset = { 0.0f, 0.0f, 0.0f };
    
public:
    CollisionComponent(Entity* owner, bool isActive) : Component(owner, isActive) {};
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
    virtual void SetCenterOffset(const DirectX::XMFLOAT3& offset) { this->centerOffset = offset; }

    virtual bool GetBounds2D(Rect2D& outRect) const { return false; }

    CollisionShape GetShape() const { return this->shape; };
    void SetShape(CollisionShape shape) { this->shape = shape; };
};
