#include "ai/ai_manager.hpp"
#include "components/ai/enemy_controller_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include <algorithm>

AIManager::AIManager()
    : spatialGrid(2.0f, 100, 100, { -100.0f, -100.0f })
{
}

bool AIManager::Initialize()
{
    this->enemies.clear();
    return true;
}

void AIManager::Shutdown()
{
    this->enemies.clear();
}

void AIManager::RegisterEnemy(EnemyControllerComponent* enemy)
{
    if (enemy == nullptr)
    {
        return;
    }

    auto it = std::find(this->enemies.begin(), this->enemies.end(), enemy);
    if (it == this->enemies.end())
    {
        this->enemies.push_back(enemy);
    }
}

void AIManager::QueueUnregister(EnemyControllerComponent* enemy) 
{
    pendingRemovals.push_back(enemy);
}

void AIManager::UnregisterEnemy(EnemyControllerComponent* enemy)
{
    if (enemy == nullptr)
    {
        return;
    }

    auto it = std::find(this->enemies.begin(), this->enemies.end(), enemy);
    if (it != this->enemies.end())
    {
        this->enemies.erase(it);
    }
}

void AIManager::FixedUpdate(float deltaTime)
{

    if (!this->pendingRemovals.empty()) {
        for(auto* each : pendingRemovals)
        {
            UnregisterEnemy(each);
        }
		pendingRemovals.clear();
    }
    size_t enemyCount = this->enemies.size();
    if (enemyCount == 0)
    {
        return;
    }

    this->spatialGrid.BeginFrame(enemyCount);
    for (size_t i = 0; i < enemyCount; ++i)
    {
        auto* e = this->enemies[i];
        if (e && e->GetOwner()->IsLocallyActive()) {
            this->spatialGrid.Insert(i, e->GetPosition());
        }
    }

    for (size_t i = 0; i < enemyCount; ++i)
    {
        auto* e = this->enemies[i];
        if (e && e->GetOwner()->IsLocallyActive())
            e->UpdateAI(deltaTime, i, this->spatialGrid);
    }

    for (auto* e : pendingRemovals) UnregisterEnemy(e);
    pendingRemovals.clear();
}