#include "scene_culler.hpp"
#include "scene/component.hpp"
#include "scene/entity.hpp"

using namespace DirectX;

void SceneCuller::AddComponent(Component *component)
{
    if (!component || !component->WantsRender())
        return;

    BoundingBox bounds;
    if (component->GetWorldBounds(bounds))
    {
        if (std::find(this->cullable.begin(), this->cullable.end(), component) == this->cullable.end())
            this->cullable.push_back(component);
        return;
    }

    if (std::find(this->alwaysVisible.begin(), this->alwaysVisible.end(), component) == this->alwaysVisible.end())
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

void SceneCuller::GatherRenderCommands(std::vector<RenderView> &outViews) const
{
    for (RenderView &view : outViews)
    {
        if (view.skipFrustumCulling)
        {
            for (Component *component : this->cullable)
                if (component->IsActive() && component->GetOwner()->IsActive())
                    component->Render(view, view.queue);
        }
        else
        {
            for (Component *component : this->cullable)
            {
                if (!component->IsActive() || !component->GetOwner()->IsActive())
                    continue;

                BoundingBox bounds;
                if (component->GetWorldBounds(bounds) && view.frustum.Contains(bounds) != DISJOINT)
                    component->Render(view, view.queue);
            }
        }

        for (Component *component : this->alwaysVisible)
            if (component->IsActive() && component->GetOwner()->IsActive())
                component->Render(view, view.queue);
    }
}