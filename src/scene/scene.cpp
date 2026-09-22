#include "scene.hpp"
#include "core/logging.hpp"
#include "scene/component.hpp"

#include <memory>
#include <algorithm>

void Scene::StartPendingEntities() {
    while (!this->entitiesPendingStart.empty()) {
        std::vector<Entity *> batch;
        batch.swap(this->entitiesPendingStart);

        for (Entity *entity : batch) {
            entity->OnStart(*this->context);

            for (Component *component : entity->GetComponents())
                this->culler.AddComponent(component);

            entity->MarkStarted();
        }
    }
}

void Scene::RemovePendingEntities() {
    while (!this->entitiesPendingRemove.empty()) {
        std::vector<Entity *> batch;
        batch.swap(this->entitiesPendingRemove);

        for (Entity *entity : batch) {
            auto iter = std::find_if(
                this->entities.begin(),
                this->entities.end(),
                [&](const std::unique_ptr<Entity> &ptr) { return ptr.get() == entity; }
            );
            if (iter != this->entities.end())
                this->DestroyEntityRecursive(entity);
        }
    }
}

void Scene::RemovePendingComponents() {
    while (!this->componentsPendingRemove.empty()) {
        std::vector<Component *> batch;
        batch.swap(this->componentsPendingRemove);

        for (Component *component : batch)
            if (Entity *owner = component->GetOwner())
                owner->RemoveComponentImmediate(component);
    }
}

void Scene::DestroyEntityRecursive(Entity *entity) {
    if (!entity)
        return;

    std::vector<Entity *> childrenCopy = entity->GetChildren();
    for (Entity *child : childrenCopy)
        this->DestroyEntityRecursive(child);

    this->DestroyEntityImmediate(entity);
}

void Scene::DestroyEntityImmediate(Entity *entity) {
    if (!entity)
        return;

    this->entitiesPendingStart.erase(
        std::remove(this->entitiesPendingStart.begin(), this->entitiesPendingStart.end(), entity), 
        this->entitiesPendingStart.end()
    );

    this->componentsPendingRemove.erase(
        std::remove_if(
            this->componentsPendingRemove.begin(), 
            this->componentsPendingRemove.end(), 
            [entity](Component *component) { return component->GetOwner() == entity; }
        ),
        this->componentsPendingRemove.end()
    );

    entity->SetParent(nullptr);

    this->rootEntities.erase(
        std::remove(this->rootEntities.begin(), this->rootEntities.end(), entity), 
        this->rootEntities.end()
    );

    for (Component *component : entity->GetComponents())
        this->culler.RemoveComponent(component);

    auto iter = std::find_if(
        this->entities.begin(),
        this->entities.end(),
        [&](const std::unique_ptr<Entity> &ptr) { return ptr.get() == entity; }
    );

    if (iter == this->entities.end())
        return;

    if (entity->HasStarted())
        (*iter)->OnDestroy(*this->context);

    this->entities.erase(iter);

    this->isHierarchyDirty = true;
}

void Scene::FlattenEntityRecursive(Entity *entity) {
    this->entityUpdateOrder.push_back(entity);

    for (Entity *child : entity->GetChildren())
        this->FlattenEntityRecursive(child);
}

void Scene::RebuildEntityUpdateOrder() {
    this->entityUpdateOrder.clear();
    this->entityUpdateOrder.reserve(this->entities.size());

    for (Entity *entity : this->rootEntities)
        this->FlattenEntityRecursive(entity);

    this->isHierarchyDirty = false;
}

void Scene::DestroyComponent(Component *component) {
    if (!component || component->isPendingDestroy)
        return;

    component->isPendingDestroy = true;
    this->componentsPendingRemove.push_back(component);
}

void Scene::InitializeComponent(Component *component) {
    if (!component)
        return;

    component->OnStart(*this->context);

    //Entity *owner = component->GetOwner();
    //const bool isStatic = owner && owner->isStatic;

    this->culler.AddComponent(component);
}

void Scene::DeinitializeComponent(Component *component) {
    if (!component)
        return;

    this->culler.RemoveComponent(component);

    component->OnDestroy(*this->context);
}

void Scene::OnEntityParentChanged(Entity *entity) {
    auto iter = std::find_if(
        this->entities.begin(),
        this->entities.end(),
        [&](const std::unique_ptr<Entity> &ptr) { return ptr.get() == entity; }
    );
    if (iter == this->entities.end())
        return;

    this->isHierarchyDirty = true;

    if (entity->GetParent()) {
        this->rootEntities.erase(std::remove(this->rootEntities.begin(), this->rootEntities.end(), entity), this->rootEntities.end());
        return;
    }

    if (std::find(this->rootEntities.begin(), this->rootEntities.end(), entity) == this->rootEntities.end())
        this->rootEntities.push_back(entity);
}

void Scene::Update(float deltaTime) {
    // TODO: Should this follow the hierarchy? Currently entities are started in the order that they are added
    this->StartPendingEntities();

    if (this->isHierarchyDirty)
        this->RebuildEntityUpdateOrder();

    for (Entity *entity : this->entityUpdateOrder)
        if (entity->IsActive())
            entity->Update(*this->context, deltaTime);

    this->RemovePendingComponents();
    this->RemovePendingEntities();
}

void Scene::GatherRenderCommands(std::vector<RenderView> &outViews) {
    this->culler.GatherRenderCommands(outViews);
}

void Scene::Clear() {
    for (auto &entity : this->entities)
        if (entity->HasStarted())
            entity->OnDestroy(*this->context);

    this->culler.Clear();

    this->entitiesPendingStart.clear();
    this->entitiesPendingRemove.clear();
    this->componentsPendingRemove.clear();

    this->rootEntities.clear();

    this->entityUpdateOrder.clear();
    this->isHierarchyDirty = false;

    this->entities.clear();

    this->name.clear();
}

Entity *Scene::AddEntity(const std::string &name, bool isActive) {
    this->entities.push_back(std::make_unique<Entity>(this, name, isActive));
    Entity *entity = this->entities.back().get();

    this->rootEntities.push_back(entity);
    this->isHierarchyDirty = true;

    this->entitiesPendingStart.push_back(entity);

    return entity;
}

void Scene::DestroyEntity(Entity *entity) {
    if (entity && std::find(this->entitiesPendingRemove.begin(), this->entitiesPendingRemove.end(), entity) == this->entitiesPendingRemove.end())
        this->entitiesPendingRemove.push_back(entity);
}

std::vector<Entity *> Scene::GetEntities() {
    std::vector<Entity *> result;
    result.reserve(this->entities.size());

    for (auto &entity : this->entities)
        result.push_back(entity.get());

    return result;
}

void Scene::SetAppContext(const AppContext &context) {
    if (this->context) {
        LogWarn("Tried to reassign the app context");
        return;
    }

    this->context = &context;
}

void Scene::PrintEntityHierarchyRecursive(Entity *entity) {
    if (!entity)
        return;

    LogInfo("%s", entity->name.c_str());

    LogIndent();
    for (Entity *child : entity->children)
        this->PrintEntityHierarchyRecursive(child);
    LogUnindent();
}

void Scene::PrintEntityHierarchy() {
    LogInfo("Entities:");

    LogIndent();
    for (Entity *entity : this->rootEntities)
        this->PrintEntityHierarchyRecursive(entity);
    LogUnindent();
}