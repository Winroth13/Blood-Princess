#include "ambient_light_component.hpp"

void AmbientLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    AmbientLightCommand command{};

    command.colour = this->colour;
    command.intensity = this->intensity;

    queue.Submit(command);
}