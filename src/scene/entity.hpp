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

    bool isStatic = false;

    std::string name;

    Entity *parent = nullptr;
    std::vector<Entity *> children;

    std::vector<std::unique_ptr<Component>> components;

    void OnComponentAdded(Component *component);
    void OnComponentRemoved(Component *component);

    friend class Scene;
    void RemoveComponentImmediate(Component *component);

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
        this->OnComponentAdded(ptr);
        return ptr;
    }

    Component *AddComponentRaw(Component *component);

    template <typename T>
    void RemoveComponent() {
        for (auto &component : this->components) {
            if (!component->IsPendingDestroy() && dynamic_cast<T *>(component.get())) {
                this->RemoveComponent(component.get());
                return;
            }
        }
    }

    void RemoveComponent(Component *component);

    // TODO: type_index
    template <typename T>
    T *GetComponent() {
        for (auto &component : this->components) {
            if (component->IsPendingDestroy())
                continue;

            if (T *ptr = dynamic_cast<T *>(component.get()))
                return ptr;
        }

        return nullptr;
    }

    std::vector<Component *> GetComponents();

    void SetParent(Entity *parent);
    Entity *GetParent() const { return this->parent; }
    const std::vector<Entity *> &GetChildren() const { return this->children; }

    bool IsActive();
    bool IsLocallyActive() const { return this->isActive; }
    void SetLocallyActive(bool isActive) { this->isActive = isActive; }

    void MarkStarted() { this->hasStarted = true; }
    bool HasStarted() const{ return this->hasStarted; }

    bool IsStatic() const { return this->isStatic; }
    void SetStatic(bool isStatic) { this->isStatic = isStatic; }

    const std::string &GetName() const { return this->name; }
    void SetName(const std::string &name) { this->name = name; }

    Scene *GetScene() const { return this->scene; }
};