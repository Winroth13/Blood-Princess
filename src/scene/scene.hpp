#pragma once

#include "core/app_context.hpp"
#include "scene/entity.hpp"
#include "scene/scene_culler.hpp"
#include "rendering/render_view.hpp"
#include "ui/ui_context.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class Entity;
class Component;

class Scene {

    std::unordered_map<std::string, std::vector<Entity*>> entityGroup;
    std::vector<std::unique_ptr<Entity>> entities;

    std::vector<Entity *> rootEntities;
    std::vector<Entity *> entityUpdateOrder;
    bool isHierarchyDirty = false;

    std::vector<Entity *> entitiesPendingStart;
    std::vector<Entity *> entitiesPendingRemove;

    std::vector<Component *> componentsPendingRemove;

    SceneCuller culler;
    UI::Context ui;

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

    void PrintEntityHierarchyRecursive(Entity *entity);

public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene &other) = delete;
    Scene &operator=(const Scene &other) = delete;

    void Update(float deltaTime, float fixedDeltaTime, int numFixedTicks);

    void GatherRenderCommands(std::vector<RenderView> &outViews);
    void GatherUICommands(uint32_t width, uint32_t height);

    void Clear();

    Entity *AddEntity(const std::string &name, bool isActive = true);
    void DestroyEntity(Entity *entity);

    std::vector<Entity *> GetEntities();
    const std::vector<Entity *> &GetRootEntities() const { return this->rootEntities; }

    const std::string &GetName() const { return this->name; }
    void SetName(const std::string &name) { this->name = name; }

    UI::Context &GetUI() { return this->ui; }

    void SetAppContext(const AppContext &context);

    void PrintEntityHierarchy();

    // Groups functions
    void AddEntityToGroup(const std::string& groupName, Entity* entity);
    void RemoveEntityFromGroup(const std::string& groupName, Entity* entity);
    void RemoveEntityFromAllGroups(Entity* entity);
    Entity* GetFirstEntityInGroup(const std::string& groupName) const;
    void ClearGroup(const std::string& groupName);
};