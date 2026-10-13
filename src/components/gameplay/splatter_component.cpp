#include "components/gameplay/splatter_component.hpp"
#include "collision/collision_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "core/rng.hpp"
#include "core/logging.hpp"
#include "scene/scene.hpp"




void SplatterComponent::OnStart(const AppContext& context)
{
    this->hitEventId = EventBus::Subscribe<CollisionEvent>(
        this, &SplatterComponent::OnCollisionEvent
    );
}

void SplatterComponent::Update(const AppContext& context, float deltaTime)
{
}

void SplatterComponent::OnDestroy(const AppContext& context)
{
    if (this->hitEventId != 0) {
        EventBus::Unsubscribe<CollisionEvent>(this->hitEventId);
        this->hitEventId = 0;
    }
}

void SplatterComponent::Reset()
{
    this->hasPaintedOnce = false;
    this->lastPaintPos = { 0.0f, 0.0f, 0.0f };
}

void SplatterComponent::OnCollisionEvent(const CollisionEvent& e)
{
    if (!e.selfCollider || e.selfCollider->GetOwner() != this->GetOwner())
        return;

    if (!e.otherCollider || !e.otherCollider->GetOwner())
        return;

    Entity* otherOwner = e.otherCollider->GetOwner();
    if (!otherOwner->IsActive())
        return;


    PaintableComponent* paintable = otherOwner->GetComponent<PaintableComponent>();
    if (!paintable)
        return;

    TransformComponent* tr = this->GetOwner()->GetComponent<TransformComponent>();
    if (!tr)
        return;

    const DirectX::XMFLOAT3 now = tr->GetWorldPosition();

    if (this->hasPaintedOnce)
    {
        const float dx = now.x - this->lastPaintPos.x;
        const float dz = now.z - this->lastPaintPos.z;
        const float distSq = dx * dx + dz * dz;

        if (distSq < this->lastPaintDist * this->lastPaintDist)
            return;
    }

    paintable->PaintAtPosition(now, RNG::Float(0.2f, 0.6f));

    this->lastPaintPos = now;
    this->hasPaintedOnce = true;
}

void SplatterComponent::PaintFloorAtPosition(const DirectX::XMFLOAT3& position, float radius)
{
    Scene* scene = this->GetOwner()->GetScene();
    if (!scene) return;

    Entity* floorRoot = nullptr;
    for (Entity* root : scene->GetRootEntities())
    {
        if (root && root->GetName() == "Floor")
        {
            floorRoot = root;
            break;
        }
    }
    if (!floorRoot) return;

    const float halfTile = 5.0f;

    for (Entity* tileEntity : floorRoot->GetChildren())
    {
        if (!tileEntity || !tileEntity->IsActive()) continue;

        PaintableComponent* tilePaintable = tileEntity->GetComponent<PaintableComponent>();
        if (!tilePaintable) continue;

        TransformComponent* tr = tileEntity->GetComponent<TransformComponent>();
        if (!tr) continue;

        DirectX::XMFLOAT3 tilePos = tr->GetWorldPosition();

        float minX = tilePos.x - halfTile;
        float maxX = tilePos.x + halfTile;
        float minZ = tilePos.z - halfTile;
        float maxZ = tilePos.z + halfTile;

        float closestX = (std::max)(minX, (std::min)(position.x, maxX));
        float closestZ = (std::max)(minZ, (std::min)(position.z, maxZ));

        float dx = position.x - closestX;
        float dz = position.z - closestZ;

        if (dx * dx + dz * dz <= radius * radius)
            tilePaintable->PaintAtPosition(position, radius);
    }
}
