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

void EnemyControllerComponent::UpdateAI(float deltaTime, size_t myGridIndex,
    const SpatialGrid& grid)
{
    if (!this->transform) return;

    XMFLOAT3 myPosF = this->transform->GetWorldPosition();
    XMVECTOR myPos = XMLoadFloat3(&myPosF);

    Entity* playerEnt =
        this->GetOwner()->GetScene()->GetFirstEntityInGroup("players");
    if (!playerEnt) return;
    TransformComponent* playerT = playerEnt->GetComponent<TransformComponent>();
    if (!playerT) return;

    XMFLOAT3 playerPosF = playerT->GetWorldPosition();
    XMVECTOR player = XMLoadFloat3(&playerPosF);

    XMVECTOR toPlayer = XMVectorSetY(XMVectorSubtract(player, myPos), 0.0f);
    const float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));

    XMVECTOR seek = XMVectorZero();

    if (distSq > 1e-4f)
    {
        XMFLOAT3 dir;
        XMStoreFloat3(&dir, toPlayer);
        const float angleDeg = atan2f(dir.x, dir.z) * (180.0f / XM_PI);
        this->transform->SetWorldRotation(0.0f, angleDeg, 0.0f);

        if (distSq > this->stopDistance * this->stopDistance)
            seek = XMVector3Normalize(toPlayer);
    }


    XMVECTOR avoid = XMVectorSetY(grid.ComputeAvoidance(myGridIndex,
        this->avoidanceRadius),
        0.0f);


    //const float AVOID_WEIGHT = 1.0f;
    //avoid = XMVectorScale(avoid, AVOID_WEIGHT);

    XMVECTOR steer = XMVectorSetY(XMVectorAdd(seek, avoid), 0.0f);

    const float steerLen = XMVectorGetX(XMVector3Length(steer));
    if (steerLen > 1e-4f)
    {
        if (steerLen > 1.0f)
            steer = XMVectorScale(steer, 1.0f / steerLen);

        XMVECTOR newPos = XMVectorAdd(myPos,
            XMVectorScale(steer,
                this->moveSpeed * deltaTime));
        newPos = XMVectorSetY(newPos, XMVectorGetY(myPos));

        XMFLOAT3 outPos;
        XMStoreFloat3(&outPos, newPos);
        this->transform->SetWorldPosition(outPos);
    }
}