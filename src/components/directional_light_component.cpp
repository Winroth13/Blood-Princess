#include "directional_light_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

DirectionalLightComponent::DirectionalLightComponent(Entity* owner, bool isActive) : Component(owner, isActive)
{
}

void DirectionalLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (transform && this->IsActive()) {
		DirectionalLightCommand command{};
		command.color = this->color;
		command.direction = transform->GetForward();
		command.intensity = this->intensity;

		queue.Submit(command);
	}
}

DirectX::XMFLOAT3 DirectionalLightComponent::GetColor()
{
	return this->color;
}

bool DirectionalLightComponent::SetColor(DirectX::XMFLOAT3 color)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->color = color;
	return true;
}

float DirectionalLightComponent::GetIntensity()
{
	return this->intensity;
}

bool DirectionalLightComponent::SetIntensity(float intensity)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->intensity = intensity;
	return true;
}

bool DirectionalLightComponent::GeneratingShadow()
{
	return this->ProduceShadow;
}

bool DirectionalLightComponent::SetGeneratingShadow(bool state)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->ProduceShadow = state;
	return true;
}
