#include "components/gameplay/projectile_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
#include "core/logging.hpp"

void ProjectileComponent::Update(const AppContext& context, float deltaTime)
{
    if (this->projectileAliveTime <= 0.0f) return;
    this->projectileAliveTime -= deltaTime;

    if (this->isGrounded) return;

    TransformComponent* transform =
        this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform) return;

    this->velocity.y -= deltaTime * this->gravity;

    DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
    pos.x += this->velocity.x * deltaTime;
    pos.y += this->velocity.y * deltaTime;
    pos.z += this->velocity.z * deltaTime;

    if (pos.y <= this->groundY)
    {
        pos.y = this->groundY;

        if (this->velocity.y < -0.5f)
        {
            this->velocity.y = -this->velocity.y * this->restitution;

            // Horizontal friction on impact
            const float decay = std::exp(-this->friction * deltaTime);
            this->velocity.x *= decay;
            this->velocity.z *= decay;

            // Angular damping on impact too
            this->angularVelocity.x *= decay;
            this->angularVelocity.y *= decay;
            this->angularVelocity.z *= decay;
        }
        else
        {
            // Not moving down fast enough — rest on the ground
            this->velocity.y = 0.0f;

            const float decay = std::exp(-this->friction * deltaTime);
            this->velocity.x *= decay;
            this->velocity.z *= decay;

            const float speedSq =
                this->velocity.x * this->velocity.x +
                this->velocity.y * this->velocity.y +
                this->velocity.z * this->velocity.z;

            if (speedSq < this->settleSpeedSq)
            {
                this->velocity = { 0, 0, 0 };
                this->angularVelocity = { 0, 0, 0 };
                this->isGrounded = true;
            }
        }
    }

    transform->SetWorldPosition(pos);

    // Spin
    DirectX::XMFLOAT3 rot = transform->GetWorldRotation();
    rot.x += this->angularVelocity.x * deltaTime;
    rot.y += this->angularVelocity.y * deltaTime;
    rot.z += this->angularVelocity.z * deltaTime;
    transform->SetWorldRotation(rot);
}
void ProjectileComponent::Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3& angVel)
{
	this->velocity = vel;
	this->angularVelocity = angVel;
}
