#include "enemy.hpp"
#include "core/logging.hpp"

EnemyType Enemy::GetEnemyTypeFromString(const std::string &str)
{
    if (str == "goblin")
    {
        return EnemyType::GOBLIN;
    }
    else if (str == "ogre")
    {
        return EnemyType::OGRE;
    }
    else if (str == "skeleton")
    {
        return EnemyType::SKELETON;
    }
    else
    {
        LogWarn("Unsupported enemy type %s", str.c_str());
        return EnemyType::GOBLIN;
    }
}

