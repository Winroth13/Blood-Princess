#pragma once
#include "assets/asset.hpp"
#include "assets/types/enemy.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <vector>



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

class Floor : Asset
{
private:
	XMFLOAT3 hsv;
	std::vector<Wave> waves = {};

public:
	Floor() = default;
	~Floor() = default;

	XMFLOAT3 GetHSV() const { return this->hsv; }
	void SetHSV(XMFLOAT3 hsv) { this->hsv = hsv; }

	const std::vector<Wave> &GetWaves() const { this->waves; }
	void SetWaves(const std::vector<Wave> &waves) { this->waves = waves; }
};