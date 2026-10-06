#include "hit_box_component.hpp"
#include "events/event_bus.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/audio/character_audio_component.hpp"


void HitboxComponent::Update(const AppContext& context, float deltaTime)
{
	if (this->activeDuration > 0.0f && this->IsActive()) {
		this->activeTimer += deltaTime;
		if (this->activeTimer >= this->activeDuration) {
			this->Deactivate();
		}
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
	this->SetActive(false);
	this->hitEntities.clear();
}

bool HitboxComponent::CanHit(Entity* target) const
{
	if (!this->IsActive() || !target || target == this->GetOwner())
		return false;
	Entity* owner = this->GetOwner();
	if (target == owner || (owner && target == owner->GetParent()))
		return false;
	return this->hitEntities.find(target) == this->hitEntities.end();
}

void HitboxComponent::OnSuccessfulHit(Entity* victim, const DirectX::XMFLOAT3& hitPoint)
{
	if (!victim) return;
	this->hitEntities.insert(victim);

	EventBus::Publish(AttackHitEvent{
		this->GetOwner(),
		victim,
		this->damage,
		this->knockbackForce,
		hitPoint
		});

	if(this->GetOwner()->GetParent()->HasComponent<PlayerControllerComponent>())
	{
		LogInfo("Play audio  %f", this->damage);
		auto* playerAudio = this->GetOwner()->GetParent()->GetComponent<CharacterAudioComponent>();
		if (playerAudio) {
			if(this->damage == 10.0f)
			{
				playerAudio->PlayLightAttack();
			}
			else if(this->damage == 50.0f)
			{
				playerAudio->PlayHeavyAttack();
			}
		}


	}
}
