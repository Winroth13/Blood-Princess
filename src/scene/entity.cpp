#include "entity.hpp"

void Entity::OnStart(const AppContext &context) {
    for (size_t i = 0; i < this->components.size(); ++i)
        this->components[i]->OnStart(context);
}

void Entity::Update(const AppContext &context, float deltaTime) {
    for (size_t i = 0; i < this->components.size(); ++i) {
        Component *component = this->components[i].get();
        if (component->IsActive())
            component->Update(context, deltaTime);
    }
}

void Entity::OnDestroy(const AppContext &context) {
    for (size_t i = 0; i < this->components.size(); ++i)
        this->components[i]->OnDestroy(context);
}

bool Entity::IsActive() {
    return this->isActive;
}