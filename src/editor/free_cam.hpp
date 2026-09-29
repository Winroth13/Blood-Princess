#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class FreeCam
{
	float fov = XM_PIDIV2;
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };

public:
	FreeCam() = default;
	~FreeCam() = default;

	XMMATRIX GetViewMatrix();

	void SetTransformMatrix(XMMATRIX matrix);

	XMFLOAT3 GetPosition() const { return this->position; }
	void SetPosition(XMFLOAT3 position) { this->position = position; }

	XMFLOAT3 GetRotation() const { return this->rotation; }
	void SetRotation(XMFLOAT3 rotation) { this->rotation = rotation; }
};