#pragma once
#include "assets/asset.hpp"
#include "assets/types/enemy.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <vector>

struct SpikeTrapConfig
{
	uint32_t amount = 0;
	float telegraph = 1.2f;
	float armedDuration = 4.0f;
	float cooldownDuration = 2.0f;
	float retriggerCooldown = 0.5f; 
	float damage = 25.0f;
	float knockback = 3.0f;
};

struct WaveEnemy
{
	EnemyType type;
	uint32_t amount;
	float time;
};

struct Wave
{
	float completion;
	std::vector<WaveEnemy> enemies;
};

class Floor : public Asset
{
private:
	XMFLOAT4 hsv = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT3 fogColour = { 0.0f, 0.0f, 0.0f };
	float fogDensity = 1.0f;
	float fogStartDepth = 0.0f;
	float fogEndDepth = 1.0f;
	SpikeTrapConfig spikeTraps = {};
	std::vector<Wave> waves = {};

public:
	Floor() = default;
	~Floor() = default;

	XMFLOAT4 GetHSV() const { return this->hsv; }
	void SetHSV(XMFLOAT4 hsv) { this->hsv = hsv; }

	XMFLOAT3 GetFogColour() const { return this->fogColour; }
	void SetFogColour(XMFLOAT3 colour) { this->fogColour = colour; }

	float GetFogDensity() const { return this->fogDensity; }
	void SetFogDensity(float density) { this->fogDensity = density; }

	float GetFogStartDepth() const { return this->fogStartDepth; }
	void SetFogStartDepth(float depth) { this->fogStartDepth = depth; }

	float GetFogEndDepth() const { return this->fogEndDepth; }
	void SetFogEndDepth(float depth) { this->fogEndDepth = depth; }

	const SpikeTrapConfig& GetSpikeTraps() const { return this->spikeTraps; }
	void SetSpikeTraps(const SpikeTrapConfig& config) { this->spikeTraps = config; }

	const std::vector<Wave> &GetWaves() const { return this->waves; }
	void SetWaves(const std::vector<Wave> &waves) { this->waves = waves; }
};