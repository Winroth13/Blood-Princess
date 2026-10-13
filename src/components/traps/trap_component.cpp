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


	// Telegraph

	if (this->cachedTransform && this->telegraphDuration > 0.0f)
	{
		float t = this->elapsedTime / this->telegraphDuration;
		t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

		float eased = t * t * (3.0f - 2.0f * t);

		DirectX::XMFLOAT3 pos = this->restPosition;
		pos.y -= this->riseHeight * (1.0f - eased);
		this->cachedTransform->SetLocalPosition(pos);
	}


	// arm
	if (!this->isArmed && this->elapsedTime >= this->telegraphDuration)
	{
		this->isArmed = true;
		if (this->cachedTransform)
		{
			this->cachedTransform->SetLocalPosition(this->restPosition);
		}
		if (this->cachedHitbox)
		{
			this->cachedHitbox->Activate();
		}
	}


	// expire

	if (!this->isExpired &&
		this->elapsedTime >= this->telegraphDuration + this->armedDuration)
	{
		this->isExpired = true;
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

		DirectX::XMFLOAT3 pos = this->restPosition;
		pos.y -= this->riseHeight;
		this->cachedTransform->SetLocalPosition(pos);
	}
	if (this->cachedHitbox)
	{
		this->cachedHitbox->Deactivate();
	}
}
