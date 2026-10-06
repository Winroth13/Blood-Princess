#include "components/gameplay/projectile_component.hpp"
#include "components/core/transform_component.hpp"
void ProjectileComponent::Update(const AppContext& context, float deltaTime)
{
	if (projectileAliveTime <= 0.0f) return;
	this->projectileAliveTime -= deltaTime;

	if (isGrounded)return;


	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (!transform)return;


	this->velocity.y -= deltaTime * gravity;

	DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
	pos.x += velocity.x * deltaTime;
	pos.y += velocity.y * deltaTime;
	pos.z += velocity.z * deltaTime;

	if (pos.y <= groundY) {
		pos.y = groundY;
		transform->SetWorldPosition(pos);
		isGrounded = true;
		velocity = { 0,0,0 };
		return;
	}

	transform->SetWorldPosition(pos);


	DirectX::XMFLOAT3 rot = transform->GetWorldRotation();
	rot.x += angularVelocity.x * deltaTime;
	rot.y += angularVelocity.y * deltaTime;
	rot.z += angularVelocity.z * deltaTime;

	transform->SetWorldRotation(rot);
}

void ProjectileComponent::Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3& angVel)
{
	this->velocity = vel;
	this->angularVelocity = angVel;
}
