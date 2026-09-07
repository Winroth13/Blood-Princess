#pragma once

#include "core/app_context.hpp"
#include "scene/entity.hpp"

#include <vector>
#include <memory>
#include <string>

class Entity;

class Scene {
    std::vector<std::unique_ptr<Entity>> entities;

    std::string name;

public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene &other) = delete;
    Scene &operator=(const Scene &other) = delete;

    void Update(const AppContext &context, float deltaTime);

    void Clear();

    Entity *AddEntity(const std::string &name, bool isActive = true);

    std::vector<Entity *> GetEntities();
};