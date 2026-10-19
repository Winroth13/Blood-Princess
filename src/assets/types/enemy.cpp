#include "enemy.hpp"
#include "core/logging.hpp"

/*
*  Since C++ is a great language that doesn't have any native ability to 
*  convert enums to strings and vice versa these two wonderful functions exists.
*/

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
        return "goblin";
    case EnemyType::OGRE:
        return "ogre";
    case EnemyType::SKELETON:
        return "skeleton";
    default:
        return "?";
    }
}

