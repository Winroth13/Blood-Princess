#include "scene.hpp"

#include <memory>

void Scene::Update(const AppContext &context, float deltaTime) {
    for (auto &entity : this->entities)
        entity->Update(context, deltaTime);
}

void Scene::Clear() {
    this->entities.clear();

    this->name.clear();
}

Entity *Scene::AddEntity(const std::string &name, bool isActive) {
    this->entities.push_back(std::make_unique<Entity>(this, name, isActive));
    return this->entities.back().get();
}

std::vector<Entity *> Scene::GetEntities() {
    std::vector<Entity *> result;
    result.reserve(this->entities.size());

    for (auto &entity : this->entities)
        result.push_back(entity.get());

    return result;
}