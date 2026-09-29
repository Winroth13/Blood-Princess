#include "player_controller_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "input/input.hpp"
#include "input/action.hpp"
#include <algorithm>
#include "collision/collision_manager.hpp"
#include "components/audio_component.hpp"
#include "core/rng.hpp"

void PlayerControllerComponent::OnStart(const AppContext& context)
{
	this->collisionRef = context.collisionManager;

	Input::SetMouseMode(MouseMode::CAPTURED);
}

void PlayerControllerComponent::Update(const AppContext& context, float deltaTime)
{
	if (!this->canUseDash)
	{
		this->dashCooldownTimer -= deltaTime;
		if (this->dashCooldownTimer <= 0)
		{
			this->canUseDash = true;
			this->dashCooldownTimer = 1.5f;
		}
	}

	if (this->isDashing)
	{
		PerformDash(deltaTime, this->dashDistance, this->dashDuration);
	}
	else
	{
		HandleMovement(deltaTime);
		HandleCombat(deltaTime); 
	}

	HandleCameraLook(deltaTime);

}

void PlayerControllerComponent::OnDestroy(const AppContext& context)
{
	
}

void PlayerControllerComponent::GetDashDirection(XMVECTOR &direction)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	float yawRad = XMConvertToRadians(this->yaw);

	DirectX::XMVECTOR forward = XMVectorSet(sinf(yawRad), 0.0f, cosf(yawRad), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(yawRad), 0.0f, -sinf(yawRad), 0.0f);

	if (Action::IsActionDown("MoveForward")) {
		direction = XMVectorAdd(direction, forward);
	}
	if (Action::IsActionDown("MoveBackward")) {
		direction = XMVectorAdd(direction, -forward);
	}
	if (Action::IsActionDown("MoveRight")) {
		direction = XMVectorAdd(direction, right);
	}
	if (Action::IsActionDown("MoveLeft")) {
		direction = XMVectorAdd(direction, -right);
	}
	if (Action::IsActionDown("MoveForward") && Action::IsActionDown("MoveRight")) {
		direction = XMVectorAdd(direction, forward);
		direction = XMVectorAdd(direction, right);
	}
	if (Action::IsActionDown("MoveRight") && Action::IsActionDown("MoveBackward")) {
		direction = XMVectorAdd(direction, right);
		direction = XMVectorAdd(direction, -forward);
	}
	if (Action::IsActionDown("MoveBackward") && Action::IsActionDown("MoveLeft")) {
		direction = XMVectorAdd(direction, -forward);
		direction = XMVectorAdd(direction, -right);
	}
	if (Action::IsActionDown("MoveLeft") && Action::IsActionDown("MoveForward")) {
		direction = XMVectorAdd(direction, -right);
		direction = XMVectorAdd(direction, forward);
	}
}

void PlayerControllerComponent::HandleMovement(float deltaTime)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	float yawRad = XMConvertToRadians(this->yaw);

	DirectX::XMVECTOR forward = XMVectorSet(sinf(yawRad), 0.0f, cosf(yawRad), 0.0f);
	DirectX::XMVECTOR right = XMVectorSet(cosf(yawRad), 0.0f, -sinf(yawRad), 0.0f);

	DirectX::XMVECTOR moveDir = XMVectorZero();


	if (Action::IsActionDown("MoveForward")) {
		moveDir = XMVectorAdd(moveDir, forward);
	}
	if (Action::IsActionDown("MoveBackward")) {
		moveDir = XMVectorSubtract(moveDir, forward);
	}
	if (Action::IsActionDown("MoveRight")) {
		moveDir = XMVectorAdd(moveDir, right);
	}
	if (Action::IsActionDown("MoveLeft")) {
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
	if (Input::GetMouseMode() != MouseMode::CAPTURED)
		return;

	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	float deltaX = Input::GetMouseDeltaX();
	float deltaY = Input::GetMouseDeltaY();

	this->yaw += deltaX * this->mouseSensitivity.x;
	this->pitch += deltaY * this->mouseSensitivity.y;

	this->pitch = std::clamp(this->pitch, -89.9f, 89.9f);
	transform->SetLocalRotation(this->pitch, this->yaw, 0.0f);
	
}

void PlayerControllerComponent::HandleCombat(float deltaTime)
{
	if (Action::IsActionPressed("LightAttack")) {
		PerformLightAttack();
	}
	if (Action::IsActionPressed("HeavyAttack"))
	{
		PerformHeavyAttack();
	}
	if (Action::IsActionPressed("Dash") && this->canUseDash) {
		this->dashDirection = DirectX::XMVectorZero();
		GetDashDirection(this->dashDirection);

		if (!DirectX::XMVector3Equal(this->dashDirection, DirectX::XMVectorZero())) {
			this->isDashing = true;
			this->canUseDash = false;
			this->dashTimer = 0.0f;

			
			auto dashSound = GetOwner()->GetComponent<AudioComponent>();
			dashSound->SetAudio3D(false);
			dashSound->SetLooping(false);
			float genNumber = RNG::Float(0.95f, 1.05f);
			LogInfo("Pitch %f: ", genNumber);
			dashSound->SetPitch(genNumber);
			dashSound->Play();
		}
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

void PlayerControllerComponent::PerformDash(float deltaTime, float dashDistance, float dashDuration)
{

	this->dashTimer += deltaTime;

	if (this->dashTimer >= dashDuration) {
		this->isDashing = false;
		return;
	}

	auto transform = GetOwner()->GetComponent<TransformComponent>();

	DirectX::XMVECTOR moveDir = DirectX::XMVector3Normalize(this->dashDirection);

	XMFLOAT3 currentPos = transform->GetLocalPosition();
	XMVECTOR currentPosV = XMLoadFloat3(&currentPos);

	float dashSpeed = dashDistance / dashDuration;

	XMVECTOR newVel = XMVectorScale(moveDir, dashSpeed * deltaTime);
	XMVECTOR newPosV = XMVectorAdd(currentPosV, newVel);

	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, newPosV);

	transform->SetLocalPosition(newPos);
}
