#include "collision_manager.hpp"
#include "core/logging.hpp"
#include "collision/collision_component.hpp"
#include "components/collisions/box_collision_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "events/event_bus.hpp"


bool CollisionManager::CheckBoxBoxOverlap(BoxCollisionComponent* a, BoxCollisionComponent* b, CollisionHit& outHit)
{
    outHit.isColliding = false;

    if (!a || !b) {
        return false;
    }

    BoundingOrientedBox boxA = a->GetWorldOBB();
    BoundingOrientedBox boxB = b->GetWorldOBB();

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

bool CollisionManager::CheckSphereSphereOverlap(SphereCollisionComponent* a, SphereCollisionComponent* b, CollisionHit& outHit)
{

    outHit.isColliding = false;

    if (!a || !b) {
        return false;
    }

    BoundingSphere sphereA = a->GetWorldSphere();
    BoundingSphere sphereB = b->GetWorldSphere();

    if (!sphereA.Intersects(sphereB)) {
        return false;
    }

    XMVECTOR centerA = XMLoadFloat3(&sphereA.Center);
    XMVECTOR centerB = XMLoadFloat3(&sphereB.Center);

    XMVECTOR diff = DirectX::XMVectorSubtract(centerB, centerA);
    XMVECTOR distSq = DirectX::XMVector3LengthSq(diff);

    float distance;
    XMStoreFloat(&distance, distSq);

    float radiusSum = sphereA.Radius + sphereB.Radius;
    float radiusSumSq = radiusSum * radiusSum;

    float fDistSq;
    XMStoreFloat(&fDistSq, distSq);

    XMStoreFloat3(&outHit.normal, diff);
    outHit.depth = radiusSum-distance;
    outHit.collider = b;
    outHit.isColliding = true;
    return fDistSq <= radiusSumSq;;
}

bool CollisionManager::CheckSphereBoxOverlap(SphereCollisionComponent* sphere, BoxCollisionComponent* box, CollisionHit& outHit)
{
    outHit.isColliding = false;

    if (!sphere || !box) {
        return false;
    }

    DirectX::BoundingSphere worldSphere = sphere->GetWorldSphere();
    DirectX::BoundingOrientedBox worldBox = box->GetWorldOBB();

    if (!worldSphere.Intersects(worldBox)) {
        return false;
    }

    DirectX::XMVECTOR sphereCenterVec = DirectX::XMLoadFloat3(&worldSphere.Center);
    DirectX::XMVECTOR boxCenterVec = DirectX::XMLoadFloat3(&worldBox.Center);
    DirectX::XMVECTOR dWorld = DirectX::XMVectorSubtract(sphereCenterVec, boxCenterVec);

    DirectX::XMVECTOR boxOrientation = DirectX::XMLoadFloat4(&worldBox.Orientation);
    DirectX::XMVECTOR boxOrientationConjugate = DirectX::XMQuaternionConjugate(boxOrientation);

    DirectX::XMVECTOR dLocal = DirectX::XMVector3Rotate(dWorld, boxOrientationConjugate);
    DirectX::XMFLOAT3 dLocalF3;
    DirectX::XMStoreFloat3(&dLocalF3, dLocal);

    DirectX::XMFLOAT3 pLocalF3;
    pLocalF3.x = (std::max)(-worldBox.Extents.x, (std::min)(worldBox.Extents.x, dLocalF3.x));
    pLocalF3.y = (std::max)(-worldBox.Extents.y, (std::min)(worldBox.Extents.y, dLocalF3.y));
    pLocalF3.z = (std::max)(-worldBox.Extents.z, (std::min)(worldBox.Extents.z, dLocalF3.z));

    DirectX::XMFLOAT3 diffF3(dLocalF3.x - pLocalF3.x, dLocalF3.y - pLocalF3.y, dLocalF3.z - pLocalF3.z);
    float distSq = diffF3.x * diffF3.x + diffF3.y * diffF3.y + diffF3.z * diffF3.z;

    DirectX::XMVECTOR normalLocalVec;
    float depth = 0.0f;

    if (distSq > 1e-6f)
    {
        float dist = std::sqrt(distSq);
        depth = worldSphere.Radius - dist;
        if (depth <= 0.0f)
        {
            return false;
        }
        normalLocalVec = DirectX::XMVectorSet(diffF3.x / dist, diffF3.y / dist, diffF3.z / dist, 0.0f);
    }
    else
    {
        float deltaX = worldBox.Extents.x - std::abs(dLocalF3.x);
        float deltaY = worldBox.Extents.y - std::abs(dLocalF3.y);
        float deltaZ = worldBox.Extents.z - std::abs(dLocalF3.z);

        if (deltaX <= deltaY && deltaX <= deltaZ)
        {
            float sign = (dLocalF3.x >= 0.0f) ? 1.0f : -1.0f;
            normalLocalVec = DirectX::XMVectorSet(sign, 0.0f, 0.0f, 0.0f);
            depth = worldSphere.Radius + deltaX;
        }
        else if (deltaY <= deltaZ)
        {
            float sign = (dLocalF3.y >= 0.0f) ? 1.0f : -1.0f;
            normalLocalVec = DirectX::XMVectorSet(0.0f, sign, 0.0f, 0.0f);
            depth = worldSphere.Radius + deltaY;
        }
        else
        {
            float sign = (dLocalF3.z >= 0.0f) ? 1.0f : -1.0f;
            normalLocalVec = DirectX::XMVectorSet(0.0f, 0.0f, sign, 0.0f);
            depth = worldSphere.Radius + deltaZ;
        }
    }

    DirectX::XMVECTOR normalWorldVec = DirectX::XMVector3Rotate(normalLocalVec, boxOrientation);
    normalWorldVec = DirectX::XMVector3Normalize(normalWorldVec);

    DirectX::XMStoreFloat3(&outHit.normal, normalWorldVec);
    outHit.depth = depth;
    outHit.collider = box;
    outHit.isColliding = true;
    return true;
}

bool CollisionManager::CheckOverlap(CollisionComponent* a, CollisionComponent* b, CollisionHit& outHit)
{
    if (!a || !b) return false;
    if (!this->ShouldCollide(a->GetLayer(), b->GetLayer())) {
        return false;
    }
    CollisionShape shapeA = a->GetShape();
    CollisionShape shapeB = b->GetShape();

    if (shapeA == CollisionShape::BOX && shapeB == CollisionShape::BOX)
    {
        return CheckBoxBoxOverlap(
            static_cast<BoxCollisionComponent*>(a),
            static_cast<BoxCollisionComponent*>(b),
            outHit
        );
    }
    else if (shapeA == CollisionShape::SPHERE && shapeB == CollisionShape::SPHERE)
    {
        return CheckSphereSphereOverlap(
            static_cast<SphereCollisionComponent*>(a),
            static_cast<SphereCollisionComponent*>(b),
            outHit
        );
    }
    else if (shapeA == CollisionShape::SPHERE && shapeB == CollisionShape::BOX)
    {
        return CheckSphereBoxOverlap(
            static_cast<SphereCollisionComponent*>(a),
            static_cast<BoxCollisionComponent*>(b),
            outHit
        );
    }
    else if (shapeA == CollisionShape::BOX && shapeB == CollisionShape::SPHERE)
    {
        bool hit = CheckSphereBoxOverlap(
            static_cast<SphereCollisionComponent*>(b),
            static_cast<BoxCollisionComponent*>(a),
            outHit
        );
        if (hit)
        {
            outHit.normal.x *= -1.0f;
            outHit.normal.y *= -1.0f;
            outHit.normal.z *= -1.0f;
            outHit.collider = b;
        }
        return hit;
    }

    return false;
}

void CollisionManager::BuildStaticTree()
{
    this->staticQuadTree.Clear();
    if (this->staticColliders.empty())
    {
        this->isStaticTreeDirty = false;
        return;
    }

    float minX = 1e9f, maxX = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    for (CollisionComponent* staticCol : this->staticColliders)
    {
        if (staticCol)
        {
            Rect2D b;
            if (staticCol->GetBounds2D(b))
            {
                minX = (std::min)(minX, b.minX);
                maxX = (std::max)(maxX, b.maxX);
                minZ = (std::min)(minZ, b.minZ);
                maxZ = (std::max)(maxZ, b.maxZ);
            }
        }
    }

    float padding = 10.0f;
    if (minX > maxX)
    {
        minX = -100.0f; maxX = 100.0f;
        minZ = -100.0f; maxZ = 100.0f;
    }
    else
    {
        minX -= padding; maxX += padding;
        minZ -= padding; maxZ += padding;
    }

    this->staticQuadTree.Init(Rect2D(minX, maxX, minZ, maxZ));

    for (CollisionComponent* staticCol : this->staticColliders)
    {
        if (staticCol)
        {
            Rect2D b;
            if (staticCol->GetBounds2D(b))
            {
                this->staticQuadTree.Insert(staticCol, b);
            }
        }
    }

    this->isStaticTreeDirty = false;
}

void CollisionManager::ResolvePairs()
{
    
    for (CollisionComponent* dynamicCol : this->dynamicColliders)
    {
        if (dynamicCol && dynamicCol->IsActive())
        {
            if (dynamicCol->GetShape() == CollisionShape::BOX)
                static_cast<BoxCollisionComponent*>(dynamicCol)->UpdateWorldOBB();
            else if (dynamicCol->GetShape() == CollisionShape::SPHERE)
                static_cast<SphereCollisionComponent*>(dynamicCol)->UpdateWorldSphere();
        }
    }

    for (CollisionComponent* staticCol : this->staticColliders)
    {
        if (staticCol && staticCol->IsActive())
        {
            if (staticCol->GetShape() == CollisionShape::BOX)
                static_cast<BoxCollisionComponent*>(staticCol)->UpdateWorldOBB();
            else if (staticCol->GetShape() == CollisionShape::SPHERE)
                static_cast<SphereCollisionComponent*>(staticCol)->UpdateWorldSphere();
        }
    }

    if (this->isStaticTreeDirty)
    {
        this->BuildStaticTree();
    }

    std::vector<CollisionComponent*> potentialStatic;
    potentialStatic.reserve(32);

    for (CollisionComponent* dynamicCol : this->dynamicColliders)
    {
        if (!dynamicCol || !dynamicCol->IsActive()) continue;

        Entity* dynamicOwner = dynamicCol->GetOwner();
        if (!dynamicOwner || !dynamicOwner->IsActive()) continue;

        TransformComponent* dynamicTransform = dynamicOwner->GetComponent<TransformComponent>();
        if (!dynamicTransform) continue;

        Rect2D dynamicBounds;
        if (!dynamicCol->GetBounds2D(dynamicBounds)) continue;

        potentialStatic.clear();
        this->staticQuadTree.Query(dynamicBounds, potentialStatic);

        for (size_t k = 0; k < potentialStatic.size(); ++k)
        {
            CollisionComponent* staticCol = potentialStatic[k];
            if (!staticCol || !staticCol->IsActive()) continue;

            bool duplicate = false;
            for (size_t prev = 0; prev < k; ++prev)
            {
                if (potentialStatic[prev] == staticCol)
                {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;

            Entity* staticOwner = staticCol->GetOwner();
            if (!staticOwner || !staticOwner->IsActive()) continue;

            CollisionHit hit;
            if (this->CheckOverlap(dynamicCol, staticCol, hit))
            {
                if (!dynamicCol->IsTrigger() && !staticCol->IsTrigger())
                {
                    DirectX::XMFLOAT3 currentPos = dynamicTransform->GetWorldPosition();
                    currentPos.x += hit.normal.x * hit.depth;
                    currentPos.y += hit.normal.y * hit.depth;
                    currentPos.z += hit.normal.z * hit.depth;
                    dynamicTransform->SetWorldPosition(currentPos);

                    if (dynamicCol->GetShape() == CollisionShape::BOX)
                        static_cast<BoxCollisionComponent*>(dynamicCol)->UpdateWorldOBB();
                    else if (dynamicCol->GetShape() == CollisionShape::SPHERE)
                        static_cast<SphereCollisionComponent*>(dynamicCol)->UpdateWorldSphere();
                }

                dynamicCol->OnCollision(staticCol, hit.normal);
                staticCol->OnCollision(dynamicCol, DirectX::XMFLOAT3(-hit.normal.x, -hit.normal.y, -hit.normal.z));
            }
        }
    }

    
    const size_t dynamicCount = this->dynamicColliders.size();
    for (size_t i = 0; i < dynamicCount; ++i)
    {
        CollisionComponent* colA = this->dynamicColliders[i];
        if (!colA || !colA->IsActive()) continue;

        Entity* ownerA = colA->GetOwner();
        if (!ownerA || !ownerA->IsActive()) continue;

        TransformComponent* transformA = ownerA->GetComponent<TransformComponent>();
        if (!transformA) continue;

        for (size_t j = i + 1; j < dynamicCount; ++j)
        {
            CollisionComponent* colB = this->dynamicColliders[j];
            if (!colB || !colB->IsActive()) continue;

            Entity* ownerB = colB->GetOwner();
            if (!ownerB || !ownerB->IsActive()) continue;

            TransformComponent* transformB = ownerB->GetComponent<TransformComponent>();
            if (!transformB) continue;

            CollisionHit hit;
            if (this->CheckOverlap(colA, colB, hit))
            {
                if (!colA->IsTrigger() && !colB->IsTrigger())
                {
                    float halfDepth = hit.depth * 0.5f;

                    DirectX::XMFLOAT3 posA = transformA->GetWorldPosition();
                    posA.x += hit.normal.x * halfDepth;
                    posA.y += hit.normal.y * halfDepth;
                    posA.z += hit.normal.z * halfDepth;
                    transformA->SetWorldPosition(posA);

                    DirectX::XMFLOAT3 posB = transformB->GetWorldPosition();
                    posB.x -= hit.normal.x * halfDepth;
                    posB.y -= hit.normal.y * halfDepth;
                    posB.z -= hit.normal.z * halfDepth;
                    transformB->SetWorldPosition(posB);

                    if (colA->GetShape() == CollisionShape::BOX)
                        static_cast<BoxCollisionComponent*>(colA)->UpdateWorldOBB();
                    else if (colA->GetShape() == CollisionShape::SPHERE)
                        static_cast<SphereCollisionComponent*>(colA)->UpdateWorldSphere();

                    if (colB->GetShape() == CollisionShape::BOX)
                        static_cast<BoxCollisionComponent*>(colB)->UpdateWorldOBB();
                    else if (colB->GetShape() == CollisionShape::SPHERE)
                        static_cast<SphereCollisionComponent*>(colB)->UpdateWorldSphere();
                }

                colA->OnCollision(colB, hit.normal);
                colB->OnCollision(colA, DirectX::XMFLOAT3(-hit.normal.x, -hit.normal.y, -hit.normal.z));
            }
        }
    }
}


void CollisionManager::SetLayerCollision(CollisionLayer a, CollisionLayer b, bool enable)
{
    uint32_t valA = static_cast<uint32_t>(a);
    uint32_t valB = static_cast<uint32_t>(b);

    if (enable)
    {
        this->layerCollisionMatrix[valA] |= valB;
        this->layerCollisionMatrix[valB] |= valA;
    }
    else
    {
        this->layerCollisionMatrix[valA] &= ~valB;
        this->layerCollisionMatrix[valB] &= ~valA;
    }
}

bool CollisionManager::ShouldCollide(CollisionLayer a, CollisionLayer b) const
{
    uint32_t valA = static_cast<uint32_t>(a);
    uint32_t valB = static_cast<uint32_t>(b);

    auto it = this->layerCollisionMatrix.find(valA);
    if (it == this->layerCollisionMatrix.end())
    {
        return false;
    }
    return (it->second & valB) != 0;
}

void CollisionManager::SetDefaultLayerMatrix()
{
    this->layerCollisionMatrix.clear();

    // Environment collisions
    this->SetLayerCollision(CollisionLayer::ENVIRONMENT, CollisionLayer::PLAYER, true);
 
    this->SetLayerCollision(CollisionLayer::ENVIRONMENT, CollisionLayer::ENEMY, true);
    this->SetLayerCollision(CollisionLayer::ENVIRONMENT, CollisionLayer::PROJECTILE, true);

    // Player collisions
    this->SetLayerCollision(CollisionLayer::PLAYER, CollisionLayer::ENEMY, true);
    this->SetLayerCollision(CollisionLayer::PLAYER, CollisionLayer::ENEMY_ATTACK, true);
    this->SetLayerCollision(CollisionLayer::PLAYER, CollisionLayer::TRIGGER_HAZARD, true);

    // Enemy collisions
    this->SetLayerCollision(CollisionLayer::ENEMY, CollisionLayer::ENEMY, false);
    this->SetLayerCollision(CollisionLayer::ENEMY, CollisionLayer::PLAYER_ATTACK, true);
    this->SetLayerCollision(CollisionLayer::ENEMY, CollisionLayer::TRIGGER_HAZARD, true);

    // Projectile collisions
    this->SetLayerCollision(CollisionLayer::PROJECTILE, CollisionLayer::PLAYER, true);
    this->SetLayerCollision(CollisionLayer::PROJECTILE, CollisionLayer::ENEMY, true);
}


bool CollisionManager::Initialize()
{
    LogInfo("Collision Manager have been succefully initialized!");
    this->SetDefaultLayerMatrix();
    return true;
}

void CollisionManager::Shutdown()
{
    LogInfo("Collision Manager have been succefully Shutdown!");
}

void CollisionManager::FixedUpdate(float deltaTime)
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
            this->isStaticTreeDirty = true;
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
        this->isStaticTreeDirty = true;
    }
    else
    {
        this->dynamicColliders.erase(
            std::remove(this->dynamicColliders.begin(), this->dynamicColliders.end(), collider),
            this->dynamicColliders.end()
        );
    }

}

bool CollisionManager::RaycastSphere(const RayCast& ray, SphereCollisionComponent* sphere, RayHit& outHit)
{
    outHit.hasHit = false;
    if (!sphere) return false;

    DirectX::BoundingSphere worldSphere = sphere->GetWorldSphere();
    DirectX::XMVECTOR rayOrigin = DirectX::XMLoadFloat3(&ray.point);
    DirectX::XMVECTOR rayDir = DirectX::XMLoadFloat3(&ray.direction);

    float dist = 0.0f;
    if (worldSphere.Intersects(rayOrigin, rayDir, dist))
    {
        if (dist >= 0.0f && dist <= ray.distance)
        {
            outHit.hasHit = true;
            outHit.distance = dist;
            outHit.collider = sphere;

            DirectX::XMVECTOR hitPosVec = DirectX::XMVectorMultiplyAdd(rayDir, DirectX::XMVectorReplicate(dist), rayOrigin);
            DirectX::XMStoreFloat3(&outHit.point, hitPosVec);

            DirectX::XMVECTOR centerVec = DirectX::XMLoadFloat3(&worldSphere.Center);
            DirectX::XMVECTOR normalVec = DirectX::XMVectorSubtract(hitPosVec, centerVec);
            DirectX::XMVECTOR lengthSq = DirectX::XMVector3LengthSq(normalVec);

            float fLenSq = 0.0f;
            DirectX::XMStoreFloat(&fLenSq, lengthSq);
            if (fLenSq > 1e-6f)
            {
                normalVec = DirectX::XMVector3Normalize(normalVec);
            }
            else
            {
                normalVec = DirectX::XMVectorNegate(rayDir);
            }
            DirectX::XMStoreFloat3(&outHit.normal, normalVec);

            return true;
        }
    }
    return false;
}

bool CollisionManager::RaycastBox(const RayCast& ray, BoxCollisionComponent* box, RayHit& outHit)
{
    outHit.hasHit = false;
    if (!box) return false;

    DirectX::BoundingOrientedBox worldOBB = box->GetWorldOBB();
    DirectX::XMVECTOR rayOrigin = DirectX::XMLoadFloat3(&ray.point);
    DirectX::XMVECTOR rayDir = DirectX::XMLoadFloat3(&ray.direction);

    float dist = 0.0f;
    if (worldOBB.Intersects(rayOrigin, rayDir, dist))
    {
        if (dist >= 0.0f && dist <= ray.distance)
        {
            outHit.hasHit = true;
            outHit.distance = dist;
            outHit.collider = box;

            DirectX::XMVECTOR hitPosVec = DirectX::XMVectorMultiplyAdd(rayDir, DirectX::XMVectorReplicate(dist), rayOrigin);
            DirectX::XMStoreFloat3(&outHit.point, hitPosVec);

            DirectX::XMVECTOR boxCenter = DirectX::XMLoadFloat3(&worldOBB.Center);
            DirectX::XMVECTOR boxOrientation = DirectX::XMLoadFloat4(&worldOBB.Orientation);
            DirectX::XMVECTOR boxOrientationConj = DirectX::XMQuaternionConjugate(boxOrientation);

            DirectX::XMVECTOR dWorld = DirectX::XMVectorSubtract(hitPosVec, boxCenter);
            DirectX::XMVECTOR dLocal = DirectX::XMVector3Rotate(dWorld, boxOrientationConj);

            DirectX::XMFLOAT3 pLocal;
            DirectX::XMStoreFloat3(&pLocal, dLocal);

            float dx = (std::max)(0.0f, worldOBB.Extents.x - std::abs(pLocal.x));
            float dy = (std::max)(0.0f, worldOBB.Extents.y - std::abs(pLocal.y));
            float dz = (std::max)(0.0f, worldOBB.Extents.z - std::abs(pLocal.z));

            DirectX::XMVECTOR localNormal;
            if (dx <= dy && dx <= dz)
            {
                localNormal = DirectX::XMVectorSet(pLocal.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f, 0.0f);
            }
            else if (dy <= dz)
            {
                localNormal = DirectX::XMVectorSet(0.0f, pLocal.y >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
            }
            else
            {
                localNormal = DirectX::XMVectorSet(0.0f, 0.0f, pLocal.z >= 0.0f ? 1.0f : -1.0f, 0.0f);
            }

            DirectX::XMVECTOR worldNormal = DirectX::XMVector3Rotate(localNormal, boxOrientation);
            worldNormal = DirectX::XMVector3Normalize(worldNormal);
            DirectX::XMStoreFloat3(&outHit.normal, worldNormal);

            return true;
        }
    }
    return false;
}

bool CollisionManager::Raycast(const RayCast& ray, RayHit& outHit, uint32_t layerMask, bool ignoreTriggers)
{
    outHit.hasHit = false;
    outHit.distance = ray.distance;
    outHit.collider = nullptr;

    DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&ray.direction);
    DirectX::XMVECTOR lengthSq = DirectX::XMVector3LengthSq(dirVec);
    float fLenSq = 0.0f;
    DirectX::XMStoreFloat(&fLenSq, lengthSq);
    if (fLenSq < 1e-6f || ray.distance <= 0.0f)
    {
        return false;
    }

    RayCast normalizedRay = ray;
    DirectX::XMStoreFloat3(&normalizedRay.direction, DirectX::XMVector3Normalize(dirVec));

    float closestDist = normalizedRay.distance;
    RayHit closestHit;
    bool hitAny = false;

    auto checkColliders = [&](const std::vector<CollisionComponent*>& colliders)
    {
        for (CollisionComponent* col : colliders)
        {
            if (!col || !col->IsActive()) continue;
            if (ignoreTriggers && col->IsTrigger()) continue;

            Entity* owner = col->GetOwner();
            if (owner && !owner->IsActive()) continue;

            if ((static_cast<uint32_t>(col->GetLayer()) & layerMask) == 0) continue;

            RayHit tempHit;
            bool hit = false;

            if (col->GetShape() == CollisionShape::BOX)
            {
                hit = this->RaycastBox(normalizedRay, static_cast<BoxCollisionComponent*>(col), tempHit);
            }
            else if (col->GetShape() == CollisionShape::SPHERE)
            {
                hit = this->RaycastSphere(normalizedRay, static_cast<SphereCollisionComponent*>(col), tempHit);
            }

            if (hit && tempHit.distance < closestDist)
            {
                closestDist = tempHit.distance;
                closestHit = tempHit;
                hitAny = true;
            }
        }
    };

    checkColliders(this->staticColliders);
    checkColliders(this->dynamicColliders);

    if (hitAny)
    {
        outHit = closestHit;
        return true;
    }

    return false;
}

bool CollisionManager::Raycast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float maxDistance, RayHit& outHit, uint32_t layerMask, bool ignoreTriggers)
{
    RayCast ray;
    ray.point = origin;
    ray.direction = direction;
    ray.distance = maxDistance;
    return this->Raycast(ray, outHit, layerMask, ignoreTriggers);
}

bool CollisionManager::Raycast(const RayCast& ray, RayHit& outHit, CollisionLayer layerMask, bool ignoreTriggers)
{
    return this->Raycast(ray, outHit, static_cast<uint32_t>(layerMask), ignoreTriggers);
}

bool CollisionManager::Raycast(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction, float maxDistance, RayHit& outHit, CollisionLayer layerMask, bool ignoreTriggers)
{
    return this->Raycast(origin, direction, maxDistance, outHit, static_cast<uint32_t>(layerMask), ignoreTriggers);
}

void CollisionManager::Clear()
{
    this->staticColliders.clear();
    this->dynamicColliders.clear();
    this->staticQuadTree.Clear();
    this->isStaticTreeDirty = true;
}
