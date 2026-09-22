#include "music_component.hpp"
#include "core/logging.hpp"

MusicComponent::MusicComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{
	NoLateUpdate();
	NoRender();

	this->audioClip = audioClip;
	if (audioClip == nullptr) {
		LogWarn("There are not any audio provided.");
	}
}

void MusicComponent::OnStart(const AppContext& context)
{
	this->audioManagerRef = context.audioManager;
	if (!this->audioClip) {
		LogWarn("There is not a valid AudioClip provided!");
	}
	else if (this->audioManagerRef)
	{
		this->audioManagerRef->PlayMusic(this->audioClip, this->looping, this->volume, this->pitch);
	}
}

void MusicComponent::Update(const AppContext& context, float deltaTime)
{
	this->audioManagerRef = context.audioManager;
	if (!this->audioManagerRef) return;

	if (this->isAudioClipChanged) {
		this->audioManagerRef->PlayMusic(this->audioClip, this->looping, this->volume, this->pitch);
		this->isAudioClipChanged = false;
	}
	else {
		this->audioManagerRef->SetMusicVolume(this->volume);
	}

	if (this->paused) {
		this->audioManagerRef->PauseMusic();
	}
	else
	{
		this->audioManagerRef->ResumeMusic();
	}
}

void MusicComponent::OnDestroy(const AppContext& context)
{
	if (context.audioManager) {
		context.audioManager->StopMusic();
	}
	this->audioManagerRef = nullptr;
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
	if (this->audioManagerRef) {
		this->audioManagerRef->SetMusicVolume(volume);
	}
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

bool MusicComponent::IsPaused()
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
	if (this->audioManagerRef) {
		if (this->paused) {
			this->audioManagerRef->PauseMusic();
		}
		else {
			this->audioManagerRef->ResumeMusic();
		}
	}
	return true;
}

bool MusicComponent::ChangeAudioClip(std::shared_ptr<AudioClip> audioClip)
{
	this->audioClip = audioClip;
	this->isAudioClipChanged = true;

	return true;
}

