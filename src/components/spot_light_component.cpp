#include "spot_light_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

SpotLightComponent::SpotLightComponent(Entity* owner, bool isActive) : Component(owner, isActive)
{
}

void SpotLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (transform && this->IsActive())
    {
        SpotLightCommand command{};
        command.angle = this->angle;
        command.attenuation = this->attenuation;
        command.color = this->color;
        command.direction = transform->GetForward();
        command.intensity = this->intensity;
        command.position = transform->GetWorldPosition();
        command.range = this->range;

        queue.Submit(command);
    }
}

DirectX::XMFLOAT3 SpotLightComponent::GetColor()
{
    return this->color;
}

bool SpotLightComponent::SetColor(DirectX::XMFLOAT3 color)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->color = color;
    return true;
}

float SpotLightComponent::GetRange()
{
    return this->range;
}

bool SpotLightComponent::SetRange(float range)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->range = range;
    return true;
}

float SpotLightComponent::GetAttenuation()
{
    return this->attenuation;
}

bool SpotLightComponent::SetAttenuation(float attenuation)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->attenuation = attenuation;
    return true;
}

float SpotLightComponent::GetIntensity()
{
    return this->intensity;
}

bool SpotLightComponent::SetIntensity(float intensity)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->intensity = intensity;
    return true;
}

float SpotLightComponent::GetAngle()
{
    return this->angle;
}

bool SpotLightComponent::SetAngle(float angle)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->angle = angle;
    return true;
}

bool SpotLightComponent::GeneratingShadow()
{
    return this->ProduceShadow;
}

bool SpotLightComponent::SetGeneratingShadow(bool state)
{
    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if (!transform) {
        LogWarn("The Point Lighte dose not have transform!");
        return false;
    }
    this->ProduceShadow = state;
    return true;
}
