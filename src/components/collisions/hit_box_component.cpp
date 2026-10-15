#include "hit_box_component.hpp"
#include "events/event_bus.hpp"
#include "scene/scene.hpp"


void HitboxComponent::Update(const AppContext& context, float deltaTime)
{
	if (!this->activeForEver && this->activeDuration > 0.0f && this->IsActive()) {
		this->activeTimer += deltaTime;
		if (this->activeTimer >= this->activeDuration) {
			this->Deactivate();
		}
	}

	for (auto it = this->hitEntities.begin(); it != this->hitEntities.end(); )
	{
		if (it->second < 0.0f) { ++it; continue; }
		it->second -= deltaTime;
		if (it->second <= 0.0f) it = this->hitEntities.erase(it);
		else ++it;
	}
}

void HitboxComponent::Activate(float duration)
{
	this->activeDuration = duration;
	this->activeTimer = 0.0f;
	this->hitEntities.clear();
	this->SetActive(true);
}

void HitboxComponent::Deactivate()
{
	this->activeForEver = false;
	this->SetActive(false);
	this->hitEntities.clear();
}

bool HitboxComponent::CanHit(Entity* target) const
{
	if (!this->IsActive() || !target)
		return false;
	Entity* owner = this->GetOwner();
	if (target == owner || (owner && target == owner->GetParent()))
		return false;
	return this->hitEntities.find(target) == this->hitEntities.end();
}

void HitboxComponent::OnSuccessfulHit(Entity* victim, const DirectX::XMFLOAT3& hitPoint)
{
	if (!victim) return;


	// never expires this activation (single-hit behaviour).
	this->hitEntities[victim] = (this->retriggerCooldown > 0.0f)
		? this->retriggerCooldown
		: -1.0f;

	EventBus::Publish(AttackHitEvent{
		this->GetOwner(),
		victim,
		this->damage,
		this->knockbackForce,
		hitPoint
		});

	if (this->destroyOnHit)
	{
		auto scene = this->GetOwner()->GetScene();
		if (scene) {
			scene->DestroyEntity(this->GetOwner());
		}
	}
}

