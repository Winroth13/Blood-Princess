#pragma once
#include <cstdint>
#include <typeindex>
#include <DirectXMath.h>
struct Event
{
    virtual ~Event() = default;
};

struct CollisionEvent : Event
{
};

struct DamageEvent : Event
{
};

struct DeathEvent : Event
{
};

struct TriggerEnterEvent : Event
{
};

struct TriggerExitEvent : Event
{
};