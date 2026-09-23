#pragma once

#include "collision/collision_layer.hpp"

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <unordered_map>

class CollisionComponent;
class BoxCollisionComponent;
class SphereCollisionComponent;
class CapsuleCollisionComponent;

struct RayCast
{
	DirectX::XMFLOAT3 point{};
	DirectX::XMFLOAT3 direction{};
	float distance = INFINITY;
};

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

	std::unordered_map<uint32_t, uint32_t> layerCollisionMatrix;

	bool CheckBoxBoxOverlap(BoxCollisionComponent *a, BoxCollisionComponent *b, CollisionHit &outHit);
	bool CheckSphereSphereOverlap(SphereCollisionComponent *a, SphereCollisionComponent *b, CollisionHit &outHit);
	bool CheckSphereBoxOverlap(SphereCollisionComponent *sphere, BoxCollisionComponent *box, CollisionHit &outHit);
	bool CheckOverlap(CollisionComponent* a, CollisionComponent* b, CollisionHit& outHit);

	void ResolvePairs();

public:
	CollisionManager() = default;
	~CollisionManager() = default;

	CollisionManager(const CollisionManager &other) = delete;
	CollisionManager &operator=(const CollisionManager &other) = delete;

	void SetLayerCollision(CollisionLayer a, CollisionLayer b, bool enable);
	bool ShouldCollide(CollisionLayer a, CollisionLayer b) const;
	void SetDefaultLayerMatrix();

	bool Initialize();
	void Shutdown();

	void Update(float deltaTime);

	void RegisterCollider(CollisionComponent *collider);
	void UnregisterCollider(CollisionComponent *collider);
	void Clear();

	const std::vector<CollisionComponent *> &GetStaticColliders() const { return this->staticColliders; }
	const std::vector<CollisionComponent *> &GetDynamicColliders() const { return this->dynamicColliders; }
};