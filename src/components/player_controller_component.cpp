#include "player_controller_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "input/input.hpp"

void PlayerControllerComponent::OnStart(const AppContext& context)
{
}

void PlayerControllerComponent::Update(const AppContext& context, float deltaTime)
{
	if (!this->canUseDodge)
	{
		this->dodgeCooldownTimer -= deltaTime;
	}

	if (dodgeCooldownTimer <= 0)
	{
		this->canUseDodge = true;
		dodgeCooldownTimer = 3.0f;
	}

	auto transform = GetOwner()->GetComponent<TransformComponent>();

}

void PlayerControllerComponent::OnDestroy(const AppContext& context)
{
	
}

void PlayerControllerComponent::HandleMovement(float deltaTime)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	DirectX::XMFLOAT3 position = transform->GetWorldPosition();
}

void PlayerControllerComponent::HandleCameraLook(float deltaTime)
{
}

void PlayerControllerComponent::PerformLightAttack()
{
}

void PlayerControllerComponent::PerformHeavyAttack()
{
}

void PlayerControllerComponent::PerformDodge()
{
}
