#include "music.hpp"
#include "core/logging.hpp"

Music::Music(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{
	this->audioClip = audioClip;
	if (audioClip == nullptr) {
		LogWarn("There are not any audio provided.");
	}
}

void Music::OnStart(const AppContext& context)
{
	context.audioManager->PlayMusic(this->audioClip, this->looping, this->volume);
}

void Music::OnDestroy(const AppContext& context)
{
	context.audioManager->StopMusic();
}

