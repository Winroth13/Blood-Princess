#pragma once
#include "assets/asset.hpp"

#include <vector>

#include <DirectXMath.h>
using namespace DirectX;

class Sound;
class Model;

enum class AttackType
{
	Melee,
	Projectile
};

struct EnemyAttack
{
	std::string name;
	AttackType type;
	std::string animation;
	std::shared_ptr<Sound> sound = nullptr;
	float damage;
	float knockback;
	float hitDelay;
	float hitDuration;
	float range;
};

class Enemy : public Asset
{
public:
	Enemy() = default;
	~Enemy() = default;

	std::string name;
	float health = 100.0f;
	float scale = 1.0f;
	float knockbackFactor = 1.0f;

	std::shared_ptr<Model> model = nullptr;
	std::string walkAnimation = "";

	XMFLOAT3 extends = {};
	XMFLOAT3 extendsOffset = {};
	
	std::shared_ptr<Sound> hitSound = nullptr;
	std::shared_ptr<Sound> deathSound = nullptr;
	std::shared_ptr<Sound> barkSound = nullptr;

	std::vector<EnemyAttack> attacks = {};
};