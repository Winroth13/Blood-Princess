#include "entity.hpp"
#include "scene/component.hpp"
#include "scene/scene.hpp"
#include "core/logging.hpp"

void Entity::OnComponentAdded(Component *component) {
    if (this->hasStarted && this->scene)
        this->scene->InitializeComponent(component);
}

void Entity::OnComponentRemoved(Component *component) {
    if (this->hasStarted && this->scene)
        this->scene->DeinitializeComponent(component);
}

void Entity::RemoveComponentImmediate(Component *component) {
    for (auto iter = this->components.begin(); iter != this->components.end(); ++iter) {
        if (iter->get() == component) {
            this->OnComponentRemoved(component);
            this->components.erase(iter);
            return;
        }
    }
}

void Entity::OnStart(const AppContext &context) {
    for (size_t i = 0; i < this->components.size(); ++i)
        this->components[i]->OnStart(context);
}

void Entity::Update(const AppContext &context, float deltaTime) {
    for (size_t i = 0; i < this->components.size(); ++i) {
        Component *component = this->components[i].get();
        if (component->IsActive() && !component->IsPendingDestroy())
            component->Update(context, deltaTime);
    }
}

void Entity::OnDestroy(const AppContext &context) {
    for (size_t i = 0; i < this->components.size(); ++i)
        this->components[i]->OnDestroy(context);
}

// Ownership transfer; creates unique_ptr
Component *Entity::AddComponentRaw(Component *component) {
    if (!component) {
        LogWarn("Attempted to add null component to entity '%s'", this->name.c_str());
        return nullptr;
    }

    this->components.emplace_back(component);
    Component *ptr = this->components.back().get();
    this->OnComponentAdded(ptr);
    return ptr;
}

void Entity::RemoveComponent(Component *component) {
    if (!component || component->IsPendingDestroy())
        return;

    if (component->GetOwner() != this) {
        LogWarn("Tried to remove a component from an entity that does not own it\n");
        return;
    }

    if (this->scene) {
        this->scene->DestroyComponent(component);
        return;
    }

    this->RemoveComponentImmediate(component);
}

std::vector<Component *> Entity::GetComponents() {
    std::vector<Component *> result;
    result.reserve(this->components.size());

    for (auto &component : this->components)
        result.push_back(component.get());

    return result;
}

void Entity::SetParent(Entity *parent) {
    if (parent == this->parent)
        return;

    Entity *ancestor = parent;
    while (ancestor) {
        if (ancestor == this)
            return;

        ancestor = ancestor->parent;
    }

    if (this->parent) {
        std::vector<Entity *> &siblings = this->parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    this->parent = parent;

    if (parent)
        parent->children.push_back(this);

    if (this->scene)
        this->scene->OnEntityParentChanged(this);
}

bool Entity::IsActive() {
    if (!this->parent)
        return this->isActive;

    if (!this->isActive)
        return false;

    return this->parent->IsActive();
}