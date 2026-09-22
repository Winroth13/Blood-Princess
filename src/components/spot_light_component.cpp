#include "spot_light_component.hpp"
#include "components/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 512.0f;

void SpotLightComponent::Update(const AppContext &context, float deltaTime)
{
    // TODO: Create views
}

void SpotLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    SpotLightCommand command{};
    command.innerAngle = this->innerAngle;
    command.outerAngle = this->outerAngle;
    command.colour = this->colour;
    command.direction = transform->GetForward();
    command.intensity = this->intensity;
    command.position = transform->GetRenderPosition();
    command.range = this->range;
    command.castShadows = this->castShadows;

    queue.Submit(command);
}