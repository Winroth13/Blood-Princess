#pragma once
#include <string>
#include "DirectXMath.h"
class Entity;
class CollisionComponent;

struct GamePausedEvent {};
struct GameResumedEvent {};


struct DamageTakenEvent
{
    Entity* victim = nullptr;
    Entity* attacker = nullptr;
    float damage = 0.0f;
    DirectX::XMFLOAT3 hitPoint{ 0.0f, 0.0f,0.0f };
    DirectX::XMFLOAT3 hitNormal{ 0.0f, 0.0f,0.0f };
};

struct AttackHitEvent
{
    Entity* attacker = nullptr;
    Entity* victim = nullptr;
    float damage = 0.0f;
    float knockbackForce = 0.0f;
    DirectX::XMFLOAT3 hitPoint{ 0.0f, 0.0f,0.0f };
};

struct CollisionEvent
{
    CollisionComponent* selfCollider;
    CollisionComponent* otherCollider;
    DirectX::XMFLOAT3 normal{ 0.0f,0.0f,0.0f };
};
/*
ConnectionId enemyKilledConn = 0;

this->enemyKilledConn = EventBus::Subscribe<EnemyKilledEvent>(
    [this](const EnemyKilledEvent& e) {
        this->score += static_cast<int>(e.xpReward);
    }
);

this->playerDiedConn = EventBus::Subscribe<PlayerDiedEvent>(
    this, &ScoreComponent::OnPlayerDied);


EventBus::Publish(DamageTakenEvent{ this->GetOwner(), damage, nullptr });

*/