#pragma once
#include "assets/asset.hpp"

#include <vector>
#include <memory>
#include <DirectXMath.h>
using namespace DirectX;

class Sound;
class Model;

enum class EnemyType
{
	GOBLIN,
	OGRE,
	SKELETON,
	COUNT
};

constexpr int operator+(EnemyType slot) noexcept {
	return static_cast<int>(slot);
}

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

	XMFLOAT3 extends;
	XMFLOAT3 extendsOffset;
	std::string socket;
};

class Enemy : public Asset
{
public:
	Enemy() = default;
	~Enemy() = default;

	EnemyType type;
	float health = 100.0f;						// the health of the enemy
	int score = 1;								// the score given by the enemy upon death
	float scale = 1.0f;							// the scale of the enemies model
	
	float avoidanceRadius = 1.0f;
	float moveSpeed = 0.0f;

	float knockbackFactor = 1.0f;				// how knockback the enemy takes, 0 is none
	float knockbackDecay = 0.0f;
	float knockbackStunDuration = 0.0f;

	float minRange = 0.0f;
	float maxRange = 0.0f;

	std::shared_ptr<Model> model = nullptr;
	std::string walkAnimation = "";

	XMFLOAT3 extends = {};
	XMFLOAT3 extendsOffset = {};
	
	std::shared_ptr<Sound> hitSound = nullptr;
	std::shared_ptr<Sound> deathSound = nullptr;
	std::shared_ptr<Sound> barkSound = nullptr;

	std::vector<EnemyAttack> attacks = {};

	static EnemyType GetEnemyTypeFromString(const std::string &str);
	static const std::string GetStringFromEnemyType(EnemyType type);
};