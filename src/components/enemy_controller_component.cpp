#include "enemy_controller_component.hpp"
#include "components/transform_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include <DirectXMath.h>
#include "components/target_component.hpp"
#include "scene/scene.hpp"
void EnemyControllerComponent::OnStart(const AppContext& context)
{
    this->transform = this->GetOwner()->GetComponent<TransformComponent>();

    if (!this->transform) {
        LogWarn("EnemyControllerComponent on '%s' has no TransformComponent",
            this->GetOwner()->GetName().c_str());
    }

    if (context.aiManager) {
        this->aiManager = context.aiManager;
        this->aiManager->RegisterEnemy(this);
    }
    else {
        LogWarn("EnemyControllerComponent on '%s' could not find AIManager",
            this->GetOwner()->GetName().c_str());
    }

    this->GetOwner()->GetScene()->AddEntityToGroup(this->groupName, this->GetOwner());
}

void EnemyControllerComponent::OnDestroy(const AppContext & context)
{
    if (this->aiManager) {
        this->aiManager->QueueUnregister(this);
        this->aiManager = nullptr;
    }
    this->GetOwner()->GetScene()->RemoveEntityFromGroup(this->groupName, this->GetOwner());
}

DirectX::XMFLOAT3 EnemyControllerComponent::GetPosition() const
{
    if (!this->transform)
        return { 0.0f, 0.0f, 0.0f };

    return this->transform->GetWorldPosition();
}

void EnemyControllerComponent::UpdateAI(float deltaTime, size_t myGridIndex, const SpatialGrid& grid)
{
    if (!this->transform)
        return;

    XMFLOAT3 myPosF = this->transform->GetWorldPosition();
    XMVECTOR myPos = XMLoadFloat3(&myPosF);

    XMFLOAT3 playerPosition = this->GetOwner()->GetScene()->GetFirstEntityInGroup("players")->GetComponent<TransformComponent>()->GetWorldPosition();
    XMVECTOR player = XMLoadFloat3(&playerPosition);

    XMVECTOR toPlayer = XMVectorSubtract(player, myPos);
    toPlayer = XMVectorSetY(toPlayer, 0.0f);

    float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));
    XMVECTOR seek = XMVectorZero();

    if (distSq > 0.0001f) {
        XMFLOAT3 direction;
        XMStoreFloat3(&direction, toPlayer);

        float angleRadians = atan2f(direction.x, direction.z);
        float angleDegrees = angleRadians * (180.0f / XM_PI);

        this->transform->SetWorldRotation(0.0f, angleDegrees, 0.0f);

        float stopSq = this->stopDistance * this->stopDistance;

        if (distSq > stopSq) {
            seek = XMVector3Normalize(toPlayer);
        }
    }

    XMVECTOR avoid = grid.ComputeAvoidance(myGridIndex, this->avoidanceRadius);
    avoid = XMVectorSetY(avoid, 0.0f);

    float avoidLen = XMVectorGetX(XMVector3Length(avoid));
    if (avoidLen > 0.0001f) {
        avoid = XMVector3Normalize(avoid);
    }

    XMVECTOR steer = XMVectorAdd(seek, avoid);
    steer = XMVectorSetY(steer, 0.0f);

    float steerLen = XMVectorGetX(XMVector3Length(steer));

    XMVECTOR movement = XMVectorZero();
    if (steerLen > 0.0001f) {
        steer = XMVector3Normalize(steer);
        movement = XMVectorScale(steer, this->moveSpeed * deltaTime);
    }

    XMVECTOR newPosition = XMVectorAdd(myPos, movement);
    newPosition = XMVectorSetY(newPosition, XMVectorGetY(myPos));

    XMFLOAT3 outPos;
    XMStoreFloat3(&outPos, newPosition);

    this->transform->SetWorldPosition(outPos);
}
