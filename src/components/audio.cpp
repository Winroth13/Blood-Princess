#include "audio.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"


Audio::Audio(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{

}

void Audio::OnStart(const AppContext& context)
{
}

void Audio::Update(const AppContext& context, float deltaTime)
{
}

void Audio::Render(const RenderView& view, RenderQueue& queue)
{
}

void Audio::OnDestroy(const AppContext& context)
{
}
