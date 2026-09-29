#pragma once
#include <string>

class Entity;


struct GamePausedEvent {};
struct GameResumedEvent {};

/*
ConnectionId enemyKilledConn = 0;

this->enemyKilledConn = EventBus::Subscribe<EnemyKilledEvent>(
    [this](const EnemyKilledEvent& e) {
        this->score += static_cast<int>(e.xpReward);
    });

this->playerDiedConn = EventBus::Subscribe<PlayerDiedEvent>(
    this, &ScoreComponent::OnPlayerDied);


EventBus::Publish(DamageTakenEvent{ this->GetOwner(), damage, nullptr });

*/