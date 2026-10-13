#include "components/traps/trap_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "scene/entity.hpp"


void TrapComponent::OnStart(const AppContext& context)
{
	this->cachedTransform = this->GetOwner()->GetComponent<TransformComponent>();
	this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();

	if (this->cachedTransform)
	{
		this->restPosition = this->cachedTransform->GetLocalPosition();
	}
}

void TrapComponent::Update(const AppContext& context, float deltaTime)
{
	this->elapsedTime += deltaTime;

	if (!this->cachedTransform) 
	{
		this->cachedTransform = this->GetOwner()->GetComponent<TransformComponent>();
	}

	if (!this->cachedHitbox)
	{
		this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();
	}


	// arm
	if (!this->isArmed && this->elapsedTime >= this->telegraphDuration)
	{
		this->isArmed = true;
		if (this->cachedHitbox)
		{
			this->cachedHitbox->Activate();
		}
	}

	// in-place cycle
	if (this->isArmed && this->elapsedTime >= this->telegraphDuration + this->armedDuration)
	{
		this->isArmed = false;
		if (this->cachedHitbox) this->cachedHitbox->Deactivate();
	}
	if (this->elapsedTime >= this->telegraphDuration + this->armedDuration + 2.0f)
	{
		this->elapsedTime = 0.0f;
	}
}

void TrapComponent::Reset()
{
	this->elapsedTime = 0.0f;
	this->isArmed = false;
	this->isExpired = false;

	if (!this->cachedTransform)
	{
		this->cachedTransform = this->GetOwner()->GetComponent<TransformComponent>();
	}
	if (!this->cachedHitbox)
	{
		this->cachedHitbox = this->GetOwner()->GetComponent<HitboxComponent>();
	}

	if (this->cachedTransform)
	{
		this->restPosition = this->cachedTransform->GetLocalPosition();
	}
	if (this->cachedHitbox)
	{
		this->cachedHitbox->Deactivate();
	}
}
