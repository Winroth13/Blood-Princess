#pragma once
#include "scene/entity.hpp"
#include "scene/component.hpp"
#include "events/signal.hpp"

class Health : public Component
{
    RegisterComponent(Health);

    ExportProperty(float, maxHealth, 100.0f);
    ExportProperty(float, currentHealth, 100.0f);

    //int connectionId = 0;
    //Signal<float /*currentHealth*/, float /*maxHealth*/> OnChangedHealth;
    //Signal<> OnDeath;
public:
    Health(Entity* owner, bool isActive);

private:
    void OnStart(const AppContext& context) override {};
    void Update(const AppContext& context, float deltaTime) override {};
    void LateUpdate(const AppContext& context, float deltaTime) override {}
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override {};

public:

    void TakeDamage(float damage);
    bool IsDead();

    float GetMaxHealth() const { return this->maxHealth; }
    void SetMaxHealth(float amount) { this->maxHealth = amount; }

    float GetCurrentHealth() { return this->currentHealth; }
    void SetCurrentHealth(float amount) { this->currentHealth = amount; }
};