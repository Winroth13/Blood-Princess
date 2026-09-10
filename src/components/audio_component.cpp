#include "audio_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"


AudioComponent::AudioComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{

}

void AudioComponent::OnStart(const AppContext& context)
{
}

void AudioComponent::Update(const AppContext& context, float deltaTime)
{
}

void AudioComponent::Render(const RenderView& view, RenderQueue& queue)
{
}

void AudioComponent::OnDestroy(const AppContext& context)
{
}
