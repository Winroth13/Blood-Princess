#pragma once
#include "scene/entity.hpp"
#include "scene/component.hpp"
#include "events/signal.hpp"

// TODO: Should probably be HealthComponent... can't be bothered changing it though lol
class Health : public Component
{
    RegisterComponent(Health);

    ExportProperty(float, maxHealth, 100.0f);
    ExportPropertyChanged(maxHealth, OnHealthEdited);
    ExportProperty(float, currentHealth, 100.0f);
    ExportPropertyChanged(currentHealth, OnHealthEdited);

    Signal<float, float> onChangedHealth;
    Signal<> onDeath;

    void NotifyChanged(bool wasDead);

    void OnHealthEdited();

public:
    Health(Entity *owner, bool isActive) : Component(owner, isActive) {}

    void TakeDamage(float damage);
    bool IsDead() const { return this->currentHealth <= 0.0f; }

    float GetMaxHealth() const { return this->maxHealth; }
    void SetMaxHealth(float maxHealth);

    float GetCurrentHealth() const { return this->currentHealth; }
    void SetCurrentHealth(float currentHealth);

    Signal<float, float> &GetOnChangedHealth() { return this->onChangedHealth; }
    Signal<> &GetOnDeath() { return this->onDeath; }
};