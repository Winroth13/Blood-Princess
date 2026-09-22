#pragma once

#include "collision/collision_layer.hpp"

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>

class CollisionComponent;
class BoxCollisionComponent;
class SphereCollisionComponent;
class CapsuleCollisionComponent;

struct CollisionHit
{
	CollisionComponent *collider = nullptr;
	bool isColliding = false;
	DirectX::XMFLOAT3 normal{ 0.0f, 0.0f, 0.0f };
	float depth = 0.0f;
};

class CollisionManager
{
private:
	std::vector<CollisionComponent *> staticColliders;
	std::vector<CollisionComponent *> dynamicColliders;

	bool CheckBoxBoxOverlap(BoxCollisionComponent *a, BoxCollisionComponent *b, CollisionHit &outHit);

	bool CheckSphereSphereOverlap(SphereCollisionComponent *a, SphereCollisionComponent *b, CollisionHit &outHit);
	bool CheckSphereBoxOverlap(SphereCollisionComponent *sphere, BoxCollisionComponent *box, CollisionHit &outHit);

	bool CheckCapsuleCapsuleOverlap(CapsuleCollisionComponent *a, CapsuleCollisionComponent *b, CollisionHit &outHit);
	bool CheckCapsuleSphereOverlap(CapsuleCollisionComponent *capsule, SphereCollisionComponent *sphere, CollisionHit &outHit);
	bool CheckCapsuleBoxOverlap(CapsuleCollisionComponent *capsule, BoxCollisionComponent *box, CollisionHit &outHit);

	void ResolvePairs();

public:
	CollisionManager() = default;
	~CollisionManager() = default;

	CollisionManager(const CollisionManager &other) = delete;
	CollisionManager &operator=(const CollisionManager &other) = delete;

	bool Initialize();
	void Shutdown();

	void Update(float deltaTime);

	void RegisterCollider(CollisionComponent *collider);
	void UnregisterCollider(CollisionComponent *collider);
	void Clear();

	const std::vector<CollisionComponent *> &GetStaticColliders() const { return this->staticColliders; }
	const std::vector<CollisionComponent *> &GetDynamicColliders() const { return this->dynamicColliders; }
};