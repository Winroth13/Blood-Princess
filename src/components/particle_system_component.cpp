#include "particle_system_component.hpp"
#include "scene/entity.hpp"
#include "components/transform_component.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"
#include "rendering/renderer.hpp"

void ParticleSystemComponent::OnStart(const AppContext &context)
{
    if (!CreateParticleBuffer(context.renderer->GetDevice()))
    {
        LogWarn(
            "Particle %s failed to create it's particle system buffer.", 
            this->GetOwner()->GetName().c_str()
        );
    }
}

void ParticleSystemComponent::Render(const RenderView &view, RenderQueue &queue)
{
    if (view.type != ViewType::PRIMARY)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (!transform)
        return;

    ParticleSystemCommand command = {};

    command.count = this->count;
    command.position = transform->GetRenderPosition();

    command.lifeTime = this->lifeTime;
    command.spawnRadius = this->spawnRadius;
           
    command.atlasSize = this->atlasSize;
    command.animationSpeed = this->animationSpeed;
    
    XMVECTOR velocityV = XMVector3Transform(XMLoadFloat3(&this->velocity), transform->GetRenderMatrix());
    XMStoreFloat3(&command.velocity, velocityV);

    XMVECTOR accelerationV = XMVector3Transform(XMLoadFloat3(&this->acceleration), transform->GetRenderMatrix());
    XMStoreFloat3(&command.acceleration, accelerationV);
    
    command.startScale = this->startScale;
    command.endScale = this->endScale;
           
    command.startTint = this->startTint;
    command.endTint = this->endTint;

    command.srv = this->particleSystemSRV;
    command.uav = this->particleSystemUAV;

    queue.Submit(command);
}

void ParticleSystemComponent::OnDestroy(const AppContext &context)
{
    SafeRelease(this->particleSystemSRV);
    SafeRelease(this->particleSystemUAV);
    SafeRelease(this->particleSystemSB);
}

bool ParticleSystemComponent::CreateParticleBuffer(ID3D11Device *device)
{
    SafeRelease(this->particleSystemSRV);
    SafeRelease(this->particleSystemUAV);
    SafeRelease(this->particleSystemSB);

    if (!RenderUtils::CreateStructuredBuffer(
        device,
        sizeof(ParticleData),
        this->count,
        &this->particleSystemSB,
        &this->particleSystemSRV,
        &this->particleSystemUAV,
        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
        "Particle System Buffer")
    )
    {
        LogError("Failed to create Particle System Buffer");
        return false;
    }

    LogUnindent();

    return true;
}
