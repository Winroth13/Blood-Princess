#include "components/traps/trap_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/skeletal/animator_component.hpp"
#include "components/audio/sound_component.hpp"
#include "collision/collision_component.hpp"
#include "collision/collision_layer.hpp"
#include "assets/asset_manager.hpp"
#include "scene/entity.hpp"


void TrapComponent::OnStart(const AppContext& context)
{
	this->cachedTransform = this->GetOwner()->GetComponent<TransformComponent>();
	this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();
	this->cachedAnimator = this->GetOwner()->GetComponent<AnimatorComponent>();
	this->cachedSound = this->GetOwner()->GetComponent<SoundComponent>();

	this->collisionEventConn = EventBus::Subscribe<CollisionEvent>(
		this, &TrapComponent::OnCollision
	);

	this->trapTelegraph = context.assetManager->Get<Sound>("assets/sounds/trap_telegraph.sound");
	this->trapArmed = context.assetManager->Get<Sound>("assets/sounds/trap_armed.sound");
	this->trapCooldown = context.assetManager->Get<Sound>("assets/sounds/trap_cooldown.sound");

}

void TrapComponent::Update(const AppContext& context, float deltaTime)
{
	switch (this->state)
	{
	case TrapState::IDLE:
		break;

	case TrapState::TELEGRAPH:
		this->timer += deltaTime;
		if (this->timer >= this->telegraphDuration)
		{
			this->TriggerArmed();

		}
		break;
	case TrapState::ARMED:
		this->timer += deltaTime;
		if (this->timer >= this->armedDuration)
		{
			if (!this->cachedHitbox)
				this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();
			if (this->cachedHitbox)
				this->cachedHitbox->Deactivate();
			if (!this->cachedAnimator)
				this->cachedAnimator = this->GetOwner()->GetComponent<AnimatorComponent>();

			this->state = TrapState::COOLDOWN;
			this->cachedSound->SetSound(this->trapCooldown);
			this->cachedSound->Play();

			this->cachedAnimator->CrossFade("spikedown", 0, false);
			this->timer = 0.0f;
		}
		break;
	case TrapState::COOLDOWN:
		this->timer += deltaTime;
		if (this->timer >= this->cooldownDuration)
		{
			this->state = TrapState::IDLE;
			this->timer = 0.0f;
		}
		break;
	}
}

void TrapComponent::OnDestroy(const AppContext& context)
{
	if (this->collisionEventConn != 0)
	{
		EventBus::Unsubscribe<CollisionEvent>(this->collisionEventConn);
		this->collisionEventConn = 0;
	}
}

void TrapComponent::OnCollision(const CollisionEvent & e)
{
	if (this->state != TrapState::IDLE)
		return;

	if (!e.selfCollider || e.selfCollider->GetOwner() != this->GetOwner())
		return;

	if (!e.otherCollider || !e.otherCollider->GetOwner())
		return;


	CollisionLayer otherLayer = e.otherCollider->GetLayer();

	if (otherLayer != CollisionLayer::PLAYER && otherLayer != CollisionLayer::ENEMY)
		return;


	if (this->telegraphDuration > 0.0f)
	{
		this->cachedSound->SetSound(this->trapTelegraph);
		this->cachedSound->Play();


		this->state = TrapState::TELEGRAPH;
		this->timer = 0.0f;
		this->cachedAnimator->Play("spiketease", false);
	}
	else
	{
		this->TriggerArmed();
	}
}

void TrapComponent::TriggerArmed()
{
	this->state = TrapState::ARMED;
	this->timer = 0.0f;

	if (!this->cachedHitbox)
		this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();

	if (this->cachedHitbox)
		this->cachedHitbox->Activate();

	if (!this->cachedAnimator)
		this->cachedAnimator = this->GetOwner()->GetComponent<AnimatorComponent>();

	if (this->cachedAnimator)
	{
		if (this->cachedAnimator->HasAnimation("spikeup"))
		{
			this->cachedSound->SetSound(this->trapArmed);
			this->cachedSound->Play();

			this->cachedAnimator->CrossFade("spikeup", 0, false);
		}
	}
}

void TrapComponent::Reset()
{
	this->state = TrapState::IDLE;
	this->timer = 0.0f;
	this->isExpired = false;

	if (!this->cachedHitbox)
		this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();

	if (this->cachedHitbox)
		this->cachedHitbox->Deactivate();

	if (!this->cachedAnimator)
		this->cachedAnimator = this->GetOwner()->GetComponent<AnimatorComponent>();

	if (this->cachedAnimator)
		this->cachedAnimator->Stop();
}
