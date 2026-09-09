#include "scene_culler.hpp"
#include "scene/component.hpp"
#include "scene/entity.hpp"

using namespace DirectX;

void SceneCuller::AddComponent(Component *component)
{
    if (!component)
        return;

    BoundingBox bounds;
    if (component->GetWorldBounds(bounds))
    {
        if (std::find(this->cullable.begin(), this->cullable.end(), component) != this->cullable.end())
            this->cullable.push_back(component);
        return;
    }

    if (std::find(this->alwaysVisible.begin(), this->alwaysVisible.end(), component) != this->alwaysVisible.end())
        this->alwaysVisible.push_back(component);
}

void SceneCuller::RemoveComponent(Component *component)
{
    if (!component)
        return;

    this->cullable.erase(
        std::remove(this->cullable.begin(), this->cullable.end(), component),
        this->cullable.end()
    );

    this->alwaysVisible.erase(
        std::remove(this->alwaysVisible.begin(), this->alwaysVisible.end(), component),
        this->alwaysVisible.end()
    );
}

void SceneCuller::Clear()
{
    this->cullable.clear();
    this->alwaysVisible.clear();
}

void SceneCuller::GatherVisibility(std::vector<RenderView> &outViews) const
{
    for (RenderView &view : outViews)
    {
        std::vector<Component *> visible;

        if (view.skipFrustumCulling)
        {
            for (Component *component : this->cullable)
                if (component->IsActive() && component->GetOwner()->IsActive())
                    visible.push_back(component);
        }
        else
        {
            for (Component *component : this->cullable)
            {
                if (!component->IsActive() || !component->GetOwner()->IsActive())
                    continue;

                BoundingBox bounds;
                if (component->GetWorldBounds(bounds) && view.frustum.Contains(bounds) != DISJOINT)
                    visible.push_back(component);
            }
        }

        for (Component *component : this->alwaysVisible)
            if (component->IsActive() && component->GetOwner()->IsActive())
                visible.push_back(component);

        for (Component *component : visible)
            component->Render(view, view.queue);
    }
}