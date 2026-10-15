#include "components/gameplay/projectile_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"
#include "core/logging.hpp"
#include "scene/scene.hpp"
#include <cmath>

void ProjectileComponent::Update(const AppContext& context, float deltaTime)
{
    if (this->projectileAliveTime <= 0.0f)
    {
        auto scene = this->GetOwner()->GetScene();
        if (scene) {
            scene->DestroyEntity(this->GetOwner());
        }
        return;
    }
    this->projectileAliveTime -= deltaTime;

    if (this->isGrounded)
    {
        if (this->destroyOnGrounded)
        {
            auto scene = this->GetOwner()->GetScene();
            if (scene) {
                scene->DestroyEntity(this->GetOwner());
            }
        }
        return;
    }

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

        if (this->velocity.y < -0.5f && this->restitution > 0.0f)
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

            if (speedSq < this->settleSpeedSq || this->restitution <= 0.0f)
            {
                this->velocity = { 0, 0, 0 };
                this->angularVelocity = { 0, 0, 0 };
                this->isGrounded = true;

                if (this->destroyOnGrounded)
                {
                    auto scene = this->GetOwner()->GetScene();
                    if (scene) {
                        scene->DestroyEntity(this->GetOwner());
                    }
                    return;
                }
            }
        }
    }

    transform->SetWorldPosition(pos);

    // Orient towards velocity if moving (model tip points along local -X)
    float speedSq = this->velocity.x * this->velocity.x + this->velocity.y * this->velocity.y + this->velocity.z * this->velocity.z;
    if (speedSq > 0.01f)
    {
        DirectX::XMVECTOR velVec = DirectX::XMLoadFloat3(&this->velocity);
        DirectX::XMVECTOR f = DirectX::XMVector3Normalize(velVec);

        DirectX::XMVECTOR row0 = DirectX::XMVectorNegate(f);

        DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        if (std::abs(DirectX::XMVectorGetY(f)) > 0.99f)
        {
            worldUp = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        }

        DirectX::XMVECTOR row2 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(row0, worldUp));
        DirectX::XMVECTOR row1 = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(row2, row0));

        DirectX::XMMATRIX rotMat;
        rotMat.r[0] = row0;
        rotMat.r[1] = row1;
        rotMat.r[2] = row2;
        rotMat.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationMatrix(rotMat);
        DirectX::XMFLOAT4 quat;
        DirectX::XMStoreFloat4(&quat, q);
        transform->SetWorldRotationQuaternion(quat);
    }
    else
    {
        // Spin
        DirectX::XMFLOAT3 rot = transform->GetWorldRotation();
        rot.x += this->angularVelocity.x * deltaTime;
        rot.y += this->angularVelocity.y * deltaTime;
        rot.z += this->angularVelocity.z * deltaTime;
        transform->SetWorldRotation(rot);
    }
}

void ProjectileComponent::Launch(const DirectX::XMFLOAT3& vel, const DirectX::XMFLOAT3& angVel)
{
	this->velocity = vel;
	this->angularVelocity = angVel;

    this->isGrounded = false;

    if (!this->hasCustomGroundY)
    {
        TransformComponent* transform = this->GetOwner()->GetComponent<TransformComponent>();
        if (transform) {
            this->groundY = transform->GetWorldPosition().y - 2.0f;
        }
        else {
            this->groundY = -2.0f;
        }
    }
}
