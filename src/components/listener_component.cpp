#include "listener_component.hpp"
#include "core/logging.hpp"

void ListenerComponent::OnStart(const AppContext& context)
{
	this->audioManagerRef = context.audioManager;
}

void ListenerComponent::Update(const AppContext& context, float deltaTime)
{
	if (auto transform = this->GetOwner()->GetComponent<TransformComponent>()) {
		this->audioManagerRef->SetListenerPosition(
			transform->GetWorldPosition()
		);
		LogInfo("Listener position: %.2f, %.2f, %.2f", transform->GetWorldPosition().x, transform->GetWorldPosition().y, transform->GetWorldPosition().z);
	}
}
