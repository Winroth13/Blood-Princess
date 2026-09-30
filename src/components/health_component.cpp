#include "health_component.hpp"

Health::Health(Entity* owner, bool isActive)
	: Component(owner, isActive)
{ 
	NoUpdate(), NoLateUpdate(), NoRender();
}
void Health::TakeDamage(float damage)
{
	if (damage <= 0.0f || IsDead()) return;

	currentHealth = (0.0f > this->currentHealth - damage) ? (0.0f) : (this->currentHealth - damage);

	//OnChangedHealth.Emit(this->currentHealth, this->maxHealth);

	if (IsDead()) {
		//OnDeath.Emit();
	}
}

bool Health::IsDead()
{
	return currentHealth <= 0.0f;;
}

