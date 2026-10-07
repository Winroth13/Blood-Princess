#include "listener_component.hpp"
#include "core/logging.hpp"

void ListenerComponent::OnStart(const AppContext& context)
{
	this->audioManagerRef = context.audioManager;
	if (this->audioManagerRef) {
		if (auto transform = this->GetOwner()->GetComponent<TransformComponent>()) {
			this->audioManagerRef->SetListener(
				transform->GetWorldPosition(),
				transform->GetForward(),
				transform->GetUp()
			);
		}
	}
}

void ListenerComponent::Update(const AppContext& context, float deltaTime)
{
	if (this->audioManagerRef) {
		if (auto transform = this->GetOwner()->GetComponent<TransformComponent>()) {
			this->audioManagerRef->SetListener(
				transform->GetWorldPosition(),
				transform->GetForward(),
				transform->GetUp()
			);
		}
	}
}
