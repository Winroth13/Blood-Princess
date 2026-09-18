#include "collision_manager.hpp"

bool CollisionManager::CheckBoxOverlap(BoxCollisionComponent* a, BoxCollisionComponent* b, CollisionHit& outHit)
{
    return false;
}

bool CollisionManager::CheckSphereOverlap(SphereCollisionComponent* a, SphereCollisionComponent* b, CollisionHit& outHit)
{
    return false;
}

bool CollisionManager::CheckCapsuleOverlap(CapsuleCollisionComponent* a, CapsuleCollisionComponent* b, CollisionHit& outHit)
{
    return false;
}

void CollisionManager::ResolvePairs()
{}

bool CollisionManager::Initialize()
{
    return false;
}

void CollisionManager::Shutdown()
{}

void CollisionManager::Update(float deltaTime)
{}

void CollisionManager::RegisterCollider(CollisionComponent * collider)
{}

void CollisionManager::UnregisterCollider(CollisionComponent * collider)
{}

void CollisionManager::Clear()
{}
