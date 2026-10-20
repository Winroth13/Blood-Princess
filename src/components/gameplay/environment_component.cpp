#include "environment_component.hpp"
#include "rendering/shared_resources.hpp"

void EnvironmentComponent::Render(const RenderView &view, RenderQueue &queue)
{
    EnvironmentCommand environmentCommand{};

    environmentCommand.hsvShift = this->hsvShift;
    environmentCommand.fogStartDepth = this->fogStartDepth;
    environmentCommand.fogEndDepth = this->fogEndDepth;

    uint32_t flags = 0;

    if (this->textureCube)
    {
        SkyboxCommand skyboxCommand{};

        skyboxCommand.skyboxSRV = this->textureCube->GetShaderResourceView();

        queue.Submit(skyboxCommand);

        flags |= +EnvironmentFlags::HAS_SKYBOX;
    }

    environmentCommand.environmentFlags = flags;

    queue.Submit(environmentCommand);
}