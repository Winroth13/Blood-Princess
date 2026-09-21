#include "collision_manager.hpp"
#include "core/logging.hpp"
#include "collision/collision_component.hpp"
#include "components/box_collision_component.hpp"
#include "components/transform_component.hpp"

bool CollisionManager::CheckBoxOverlap(BoxCollisionComponent* a, BoxCollisionComponent* b, CollisionHit& outHit)
{
    outHit.isColliding = false;

    if (!a || !b) {
        return false;
    }

    BoundingOrientedBox boxA = a->GetWorldOBB();
    BoundingOrientedBox boxB = a->GetWorldOBB();

    if (!boxA.Intersects(boxB)) {
        return false;
    }

    float minAX = boxA.Center.x - boxA.Extents.x;
    float maxAX = boxA.Center.x + boxA.Extents.x;
    float minAZ = boxA.Center.z - boxA.Extents.z;
    float maxAZ = boxA.Center.z + boxA.Extents.z;

    float minBX = boxB.Center.x - boxB.Extents.x;
    float maxBX = boxB.Center.x + boxB.Extents.x;
    float minBZ = boxB.Center.z - boxB.Extents.z;
    float maxBZ = boxB.Center.z + boxB.Extents.z;

    float overlapX = (std::min)(maxAX, maxBX) - (std::max)(minAX, minBX);
    float overlapZ = (std::min)(maxAZ, maxBZ) - (std::max)(minAZ, minBZ);

    if (overlapX <= 0.0f || overlapZ <= 0.0f)
    {
        return false;
    }

    if (overlapX < overlapZ)
    {
        outHit.depth = overlapX;
        outHit.normal = (boxA.Center.x < boxB.Center.x)
            ? DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f)
            : DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        outHit.depth = overlapZ;
        outHit.normal = (boxA.Center.z < boxB.Center.z)
            ? DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f)
            : DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
    }

    outHit.collider = b;
    outHit.isColliding = true;
    return true;
}

//bool CollisionManager::CheckSphereOverlap(SphereCollisionComponent* a, SphereCollisionComponent* b, CollisionHit& outHit)
//{
//    return false;
//}
//
//bool CollisionManager::CheckCapsuleOverlap(CapsuleCollisionComponent* a, CapsuleCollisionComponent* b, CollisionHit& outHit)
//{
//    return false;
//}

void CollisionManager::ResolvePairs()
{
    for (CollisionComponent *dynamicCol : this->dynamicColliders)
    {
        if (!dynamicCol || !dynamicCol->IsActive()) {
            continue;
        }

        Entity* dynamicOwner = dynamicCol->GetOwner();
        if (!dynamicOwner || !dynamicOwner->IsActive()) {
            continue;
        }

        auto* playerBox = dynamic_cast<BoxCollisionComponent*>(dynamicCol);
        if (!playerBox)
        {
            continue;
        }

        playerBox->UpdateWorldOBB();

        TransformComponent* playerTransform = dynamicOwner->GetComponent<TransformComponent>();
        if (!playerTransform)
        {
            continue;
        }


        for (CollisionComponent* staticCol : this->staticColliders)
        {
            if (!staticCol || !staticCol->IsActive())
            {
                continue;
            }

            Entity* staticOwner = staticCol->GetOwner();
            if (!staticOwner || !staticOwner->IsActive())
            {
                continue;
            }

            auto* wallBox = dynamic_cast<BoxCollisionComponent*>(staticCol);
            if (!wallBox)
            {
                continue;
            }

            wallBox->UpdateWorldOBB();

            CollisionHit hit;
            if (this->CheckBoxOverlap(playerBox, wallBox, hit))
            {
                DirectX::XMFLOAT3 currentPos = playerTransform->GetWorldPosition();
                currentPos.x += hit.normal.x * hit.depth;
                currentPos.z += hit.normal.z * hit.depth;
                playerTransform->SetWorldPosition(currentPos);

                
                playerBox->UpdateWorldOBB();

                
                dynamicCol->OnCollision(staticCol, hit.normal);
                staticCol->OnCollision(dynamicCol, DirectX::XMFLOAT3(-hit.normal.x, -hit.normal.y, -hit.normal.z));
            }
        }
    }
}

bool CollisionManager::Initialize()
{
    LogInfo("Collision Manager have been succefully initialized!");
    return true;
}

void CollisionManager::Shutdown()
{
    LogInfo("Collision Manager have been succefully Shutdown!");
}

void CollisionManager::Update(float deltaTime)
{
    this->ResolvePairs();
}


void CollisionManager::RegisterCollider(CollisionComponent * collider)
{
    if (!collider) return;

    if (collider->IsStatic()) {
        auto iter = std::find(this->staticColliders.begin(), this->staticColliders.end(), collider);
        if (iter == this->staticColliders.end()) {
            this->staticColliders.push_back(collider);
        }
    }
    else
    {
        auto iter = std::find(this->dynamicColliders.begin(), this->dynamicColliders.end(), collider);
        if (iter == this->dynamicColliders.end()) {
            this->dynamicColliders.push_back(collider);
        }
    }
}

void CollisionManager::UnregisterCollider(CollisionComponent * collider)
{
    if (!collider) return;

    if (collider->IsStatic()) {
        this->staticColliders.erase(
            std::remove(this->staticColliders.begin(),this->staticColliders.end(), collider),
            this->staticColliders.end()
        );
    }
    else
    {
        this->dynamicColliders.erase(
            std::remove(this->dynamicColliders.begin(), this->dynamicColliders.end(), collider),
            this->dynamicColliders.end()
        );
    }

}

void CollisionManager::Clear()
{
    this->staticColliders.clear();
    this->dynamicColliders.clear();
}
