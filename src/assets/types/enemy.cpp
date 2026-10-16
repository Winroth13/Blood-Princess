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

const std::string Enemy::GetStringFromEnemyType(EnemyType type)
{
    switch (type)
    {
    case EnemyType::GOBLIN:
        return "Goblin";
    case EnemyType::OGRE:
        return "Ogre";
    case EnemyType::SKELETON:
        return "Skeleton";
    default:
        return "?";
    }
}

