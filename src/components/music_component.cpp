#include "music_component.hpp"
#include "core/logging.hpp"

MusicComponent::MusicComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{
	this->audioClip = audioClip;
	if (audioClip == nullptr) {
		LogWarn("There are not any audio provided.");
	}
}

void MusicComponent::OnStart(const AppContext& context)
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
	}
	else
	{
		context.audioManager->PlayMusic(this->audioClip, this->looping, this->volume);
	}
}

void MusicComponent::Update(const AppContext& context, float deltaTime)
{
	if (this->isAudioClipChanged) {
		context.audioManager->PlayMusic(this->audioClip, this->looping, this->volume);
		this->isAudioClipChanged = false;
	}
	if (this->paused) {
		context.audioManager->PauseMusic();
	}
	else
	{
		context.audioManager->ResumeMusic();
	}
}

void MusicComponent::OnDestroy(const AppContext& context)
{
	context.audioManager->StopMusic();
}

float MusicComponent::GetVolume()
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return 0.0f;
	}
	return this->volume;
}

bool MusicComponent::SetVolume(float volume)
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	this->volume = volume;
	return true;
}

float MusicComponent::GetPitch()
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return 0.0f;
	}
	return this->pitch;
}

bool MusicComponent::SetPitch(float pitch)
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	this->pitch = pitch;
	return true;
}

bool MusicComponent::GetLooping()
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	return this->looping;
}

bool MusicComponent::SetLooping(bool looping)
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	this->looping = looping;
	return true;
}

bool MusicComponent::GetPaused()
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	return this->paused;
}

bool MusicComponent::SetPaused(bool isPlaying)
{
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
		return false;
	}
	this->paused = isPlaying;
	return true;

}

bool MusicComponent::ChangeAudioClip(std::shared_ptr<AudioClip> audioClip)
{
	this->audioClip = audioClip;
	this->isAudioClipChanged = true;

	return true;
}

