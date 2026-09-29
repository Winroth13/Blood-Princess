#include "enemy_controller_component.hpp"
#include "components/transform_component.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include <DirectXMath.h>
#include "components/target_component.hpp"
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
}

void EnemyControllerComponent::OnDestroy(const AppContext & context)
{
    if (this->aiManager) {
        this->aiManager->UnregisterEnemy(this);
        this->aiManager = nullptr;
    }
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
    XMFLOAT3 playerPosition = this->GetOwner()->GetComponent<TargetComponent>()->GetTargetPosition();
    XMVECTOR player = XMLoadFloat3(&playerPosition);

    XMVECTOR toPlayer = XMVectorSubtract(player, myPos);
    toPlayer = XMVectorSetY(toPlayer, 0.0f);

    float distSq = XMVectorGetX(XMVector3LengthSq(toPlayer));
    float aggroSq = this->aggroRadius * this->aggroRadius;

    XMVECTOR seek = XMVectorZero();
    if (distSq < aggroSq && distSq > 0.0001f) {
        float stopSq = this->stopDistance * this->stopDistance;
        if (distSq > stopSq) {
            seek = XMVector3Normalize(toPlayer);
        }
    }

    XMVECTOR avoid = grid.ComputeAvoidance(myGridIndex, this->avoidanceRadius);

    float avoidLen = XMVectorGetX(XMVector3Length(avoid));
    if (avoidLen > 0.0001f) {
        avoid = XMVectorScale(XMVector3Normalize(avoid), 1.0f);
    }

    XMVECTOR steer = XMVectorAdd(seek, avoid);
    steer = XMVectorSetY(steer, 0.0f);

    float steerLen = XMVectorGetX(XMVector3Length(steer));
    if (steerLen > 1.0f) {
        steer = XMVectorScale(XMVector3Normalize(steer), 1.0f);
    }

    XMVECTOR newPos = XMVectorMultiplyAdd(
        steer,
        XMVectorReplicate(this->moveSpeed * deltaTime),
        myPos
    );
    newPos = XMVectorSetY(newPos, XMVectorGetY(myPos));

    XMFLOAT3 outPos;
    XMStoreFloat3(&outPos, newPos);
    this->transform->SetWorldPosition(outPos);
}
