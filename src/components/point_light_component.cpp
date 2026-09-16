#include "point_light_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

PointLightComponent::PointLightComponent(Entity* owner, bool isActive) : Component(owner, isActive)
{
}

void PointLightComponent::Render(const RenderView& view, RenderQueue& queue)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (transform && this->IsActive()) {
		PointLightCommand command{};

		command.attenuation = this->attenuation;
		command.color = this->color;
		command.intensity = this->intensity;
		command.position = transform->GetWorldPosition();
		command.radius = this->radius;

		queue.Submit(command);
	}
}

DirectX::XMFLOAT3 PointLightComponent::GetColor()
{
	return this->color;
}

bool PointLightComponent::SetColor(DirectX::XMFLOAT3 color)
{
	auto transform = this->GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->color = color;
	return true;

}

float PointLightComponent::GetIntensity()
{
	return this->intensity;
}

bool PointLightComponent::SetIntensity(float intensity)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->intensity = intensity;
	return true;
}

float PointLightComponent::GetAttenuation()
{
	return this->attenuation;
}

bool PointLightComponent::SetAttenuation()
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->attenuation = attenuation;
	return true;
}

float PointLightComponent::GetRadius()
{
	return this->radius;
}

bool PointLightComponent::SetRadius(float radius)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->radius = radius;
	return true;
}

bool PointLightComponent::GeneratingShadow()
{
	return this->ProduceShadow;
}

bool PointLightComponent::SetGeneratingShadow(bool state)
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform) {
		LogWarn("The Point Lighte dose not have transform!");
		return false;
	}
	this->ProduceShadow = state;
	return true;
}
