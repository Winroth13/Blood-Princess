#include "point_light_component.hpp"
#include "components/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 512.0f;

void PointLightComponent::Update(const AppContext &context, float deltaTime) 
{
    // TODO: Create views
}

void PointLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    PointLightCommand command{};

    command.colour = this->colour;
    command.intensity = this->intensity;
    command.position = transform->GetRenderPosition();
    command.radius = this->radius;
    command.castShadows = this->castShadows;

    queue.Submit(command);
}