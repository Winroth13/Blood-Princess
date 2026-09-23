#pragma once
#include "assets/asset.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <memory>

class Texture2D;

class ParticleSystem : public Asset
{
public:
	uint32_t count = 10;
	uint32_t flags = 0;
	float lifeTime = 2.0f;
	float spawnRadius = 1.0f;

	float animationSpeed = 1.0f;
	XMFLOAT2 atlasSize = { 1.0f, 1.0f };
	std::shared_ptr<Texture2D> atlasTexture;

	XMFLOAT3 initialVelocity = { 0.0f, 0.2f, 0.0f };
	XMFLOAT3 initialAcceleration = { 0.0f , 0.0f , 0.0f };

	float startScale = 0.1f;
	float endScale = 0.5;

	XMFLOAT3 startTint = { 1.0f, 1.0f, 1.0f };
	XMFLOAT3 endTint = { 1.0f , 1.0f , 1.0f };

	ParticleSystem() = default;
	~ParticleSystem() = default;
};