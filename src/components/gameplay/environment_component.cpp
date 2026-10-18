#include "environment_component.hpp"

void EnvironmentComponent::Render(const RenderView &view, RenderQueue &queue)
{
    EnvironmentCommand environmentCommand{};

    environmentCommand.hsvShift = this->hsvShift;

    queue.Submit(environmentCommand);

    if (this->textureCube)
    {
        SkyboxCommand skyboxCommand{};

        skyboxCommand.skyboxSRV = this->textureCube->GetShaderResourceView();

        queue.Submit(skyboxCommand);
    }
}