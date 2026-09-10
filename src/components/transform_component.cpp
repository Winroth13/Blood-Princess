#include "transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "rendering/renderer.hpp"

void TransformComponent::OnStart(const AppContext &context)
{

}

void TransformComponent::Update(const AppContext &context, float deltaTime)
{
    RenderView view{};
    view.type = ViewType::PRIMARY;
    view.index = 0;

    context.renderer->AddView(view);
}