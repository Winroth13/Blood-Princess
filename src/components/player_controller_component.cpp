#include "player_controller_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "input/input.hpp"
#include "input/action.hpp"
#include <algorithm>

void PlayerControllerComponent::OnStart(const AppContext& context)
{
}

void PlayerControllerComponent::Update(const AppContext& context, float deltaTime)
{
	if (!this->canUseDash)
	{
		this->dashCooldownTimer -= deltaTime;
	}

	if (dashCooldownTimer <= 0)
	{
		this->canUseDash = true;
		dashCooldownTimer = 3.0f;
	}

	HandleMovement(deltaTime);
	HandleCameraLook(deltaTime);
	HandleCombat(deltaTime);

}

void PlayerControllerComponent::OnDestroy(const AppContext& context)
{
	
}

void PlayerControllerComponent::HandleMovement(float deltaTime)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();

	DirectX::XMVECTOR forward = XMVectorSet(sinf(this->yaw), 0.0f, cosf(this->yaw), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(this->yaw), 0.0f, -sinf(this->yaw), 0.0f);

	DirectX::XMVECTOR moveDir = XMVectorZero();


	if (Action::IsActionDown(Actions::MOVE_FORWARD)) {
		moveDir = XMVectorAdd(moveDir, forward);
	}
	if (Action::IsActionDown(Actions::MOVE_FORWARD)) {
		moveDir = XMVectorSubtract(moveDir, forward);
	}
	if (Action::IsActionDown(Actions::MOVE_RIGHT)) {
		moveDir = XMVectorAdd(moveDir, right);
	}
	if (Action::IsActionDown(Actions::MOVE_LEFT)) {
		moveDir = XMVectorSubtract(moveDir, right);
	}

	if (!DirectX::XMVector3Equal(moveDir, XMVectorZero())) {
		moveDir = XMVector3Normalize(moveDir);

		XMFLOAT3 currentPos = transform->GetLocalPosition();
		XMVECTOR currentPosV = XMLoadFloat3(&currentPos);

		XMVECTOR newVel = XMVectorScale(moveDir, deltaTime * this->moveSpeed);
		XMVECTOR newPosV = XMVectorAdd(currentPosV, newVel);

		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, newPosV);

		transform->SetLocalPosition(newPos);
	}
}

void PlayerControllerComponent::HandleCameraLook(float deltaTime)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	float deltaX = Input::GetMouseDeltaX();
	float deltaY = Input::GetMouseDeltaY();

	this->yaw += deltaX * this->mouseSensitivity;
	this->pitch += deltaY * this->mouseSensitivity;

	this->pitch = std::clamp(this->pitch, -89.9f, 89.9f);
	transform->SetLocalRotation(this->yaw, this->pitch, 0.0f);
	
}

void PlayerControllerComponent::HandleCombat(float deltaTime)
{
	if (Action::IsActionPressed(Actions::LIGHT_ATTACK)) {
		PerformLightAttack();
	}
	if (Action::IsActionPressed(Actions::HEAVY_ATTACK))
	{
		PerformHeavyAttack();
	}
	if (Action::IsActionPressed(Actions::DASH) && this->canUseDash) {
		PerformDash();
	}
}

void PlayerControllerComponent::PerformLightAttack()
{
	LogInfo("Attack with light attack.");
}

void PlayerControllerComponent::PerformHeavyAttack()
{

	LogInfo("Attack with Heavy attack.");
}

void PlayerControllerComponent::PerformDash()
{
	LogInfo("The player has Dashed.");
}
