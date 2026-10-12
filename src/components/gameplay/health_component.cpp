#include "health_component.hpp"

#include <algorithm>

#undef min
#undef max

void Health::NotifyChanged(bool wasDead) {
    this->onChangedHealth.Emit(this->currentHealth, this->maxHealth);

    if (!wasDead && this->IsDead())
        this->onDeath.Emit();
}

void Health::OnHealthEdited() {
    this->onChangedHealth.Emit(this->currentHealth, this->maxHealth);
}

void Health::TakeDamage(float damage) {
	if (damage <= 0.0f || this->IsDead())
       return;

	this->currentHealth = std::max(this->currentHealth - damage, 0.0f);
    this->NotifyChanged(false);
}

void Health::SetMaxHealth(float maxHealth) {
    this->maxHealth = maxHealth;
    this->onChangedHealth.Emit(this->currentHealth, this->maxHealth);
}

void Health::SetCurrentHealth(float currentHealth) {
    const bool wasDead = this->IsDead();
    this->currentHealth = currentHealth;
    this->NotifyChanged(wasDead);
}
