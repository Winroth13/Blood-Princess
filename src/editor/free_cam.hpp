#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class FreeCam
{
	const float MOUSE_SENSITIVITY = 0.005f;
	const float POSITION_LERP_SPEED = 10.0f;

	const float MIN_SPEED = 2.5f;
	const float MAX_SPEED = 30.0;
	const float WHEEL_SENSITIVITY = 1.0f;

	float fov = XM_PIDIV2;
	float speed = (MIN_SPEED + MAX_SPEED) * 0.25f;
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

	XMFLOAT3 targetPosition = { 0.0f, 0.0f, 0.0f };

public:
	FreeCam() = default;
	~FreeCam() = default;

	void Update(float deltaTime);

	void UpdateRotation(float deltaTime);
	void UpdateMovement(float deltaTime);

	XMMATRIX GetViewMatrix();

	XMFLOAT3 GetPosition() const { return this->position; }
	void SetPosition(XMFLOAT3 position) { this->position = position; this->targetPosition = position; }

	XMFLOAT3 GetRotation() const { return this->rotation; }
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }
};