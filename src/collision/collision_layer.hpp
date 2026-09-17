#pragma once


#include <cstdint>

enum class CollisionLayer : uint32_t
{
	NONE = 0,
	ENVIRONMENT = 1 << 0,
	PLAYER = 1 << 1,
	ENEMY = 1 << 2,
	PLAYER_ATTACK = 1 << 3,
	ENEMY_ATTACK = 1 << 4,
	PROJECTILE = 1 << 5,
	TRIGGER_HAZARD = 1 << 6
};

inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) 
{
	return static_cast<CollisionLayer>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline CollisionLayer operator&(CollisionLayer a, CollisionLayer b)
{
	return static_cast<CollisionLayer>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline CollisionLayer operator^(CollisionLayer a, CollisionLayer b)
{
	return static_cast<CollisionLayer>(static_cast<uint32_t>(a) ^ static_cast<uint32_t>(b));
}

inline CollisionLayer operator~(CollisionLayer a)
{
	return static_cast<CollisionLayer>(~static_cast<uint32_t>(a));
}
inline CollisionLayer& operator|=(CollisionLayer &a, CollisionLayer b)
{
	a = a | b;
	return a;
}
inline CollisionLayer& operator&=(CollisionLayer &a, CollisionLayer b)
{
	a = a & b;
	return a;
}
inline CollisionLayer& operator^=(CollisionLayer &a, CollisionLayer b)
{
	a = a ^ b;
	return a;
}

inline bool HasLayer(uint32_t mask, CollisionLayer layer)
{
	return (mask & static_cast<uint32_t>(layer)) != 0;
}




