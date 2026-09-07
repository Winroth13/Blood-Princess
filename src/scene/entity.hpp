#pragma once

#include "core/app_context.hpp"
#include "scene/component.hpp"

#include <vector>
#include <memory>
#include <string>

class Scene;
class Component;

class Entity {
    Scene *scene = nullptr;

    bool isActive = true;
    bool hasStarted = false;

    std::string name;

    Entity *parent = nullptr;
    std::vector<Entity *> children;

    std::vector<std::unique_ptr<Component>> components;

public:
    Entity(Scene *scene, const std::string &name, bool isActive = true) : scene(scene), name(name), isActive(isActive) {}
    ~Entity() = default;

    Entity(const Entity &other) = delete;
    Entity &operator=(const Entity &other) = delete;

    void OnStart(const AppContext &context);
    void Update(const AppContext &context, float deltaTime);
    void OnDestroy(const AppContext &context);

    template <typename T, typename... Args>
    T *AddComponent(Args&&... args) {
        this->components.emplace_back(std::make_unique<T>(this, true, std::forward<Args>(args)...));
        T *ptr = static_cast<T *>(this->components.back().get());
        // this->OnComponentAdded(ptr);
        return ptr;
    }

    bool IsActive();
    bool IsLocallyActive() const { return this->isActive; }
    void SetLocallyActive(bool isActive) { this->isActive = isActive; }
};