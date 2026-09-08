#pragma once

#include "core/app_context.hpp"
#include "scene/entity.hpp"
#include "scene/scene_culler.hpp"

#include <vector>
#include <memory>
#include <string>

class Entity;
class Component;

class Scene {
    std::vector<std::unique_ptr<Entity>> entities;

    std::vector<Entity *> rootEntities;
    std::vector<Entity *> entityUpdateOrder;
    bool isHierarchyDirty = false;

    std::vector<Entity *> entitiesPendingStart;
    std::vector<Entity *> entitiesPendingRemove;

    std::vector<Component *> componentsPendingRemove;

    SceneCuller culler;

    const AppContext *context = nullptr;

    std::string name;

    void StartPendingEntities();
    void RemovePendingEntities();
    void RemovePendingComponents();

    void DestroyEntityRecursive(Entity *entity);
    void DestroyEntityImmediate(Entity *entity);

    void FlattenEntityRecursive(Entity *entity);
    void RebuildEntityUpdateOrder();

    friend class Entity;
    void DestroyComponent(Component *component);
    void InitializeComponent(Component *component);
    void DeinitializeComponent(Component *component);
    void OnEntityParentChanged(Entity *entity);

public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene &other) = delete;
    Scene &operator=(const Scene &other) = delete;

    void Update(float deltaTime);

    // GatherVisibility() or smth, bad name idk

    void Clear();

    Entity *AddEntity(const std::string &name, bool isActive = true);
    void DestroyEntity(Entity *entity);

    std::vector<Entity *> GetEntities();
    const std::vector<Entity *> &GetRootEntities() const { return this->rootEntities; }

    const std::string &GetName() const { return this->name; }
    void SetName(const std::string &name) { this->name = name; }

    void SetAppContext(const AppContext &context);
};