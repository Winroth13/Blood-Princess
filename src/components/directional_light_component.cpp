#include "directional_light_component.hpp"
#include "components/transform_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

void DirectionalLightComponent::Update(const AppContext &context, float deltaTime) {
    // TODO: Create views
    // TODO: Cache viewProjectionMatrix
}

void DirectionalLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

	DirectionalLightCommand command{};

	command.colour = this->colour;
	command.direction = transform->GetForward();
	command.intensity = this->intensity;
    command.viewProjectionMatrix = this->viewProjectionMatrix;

	queue.Submit(command);
}