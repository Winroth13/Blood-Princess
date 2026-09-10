#include "audio_component.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"
#include "assets/asset_manager.hpp"

AudioComponent::AudioComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip)
	: Component(owner, isActive)
{
	this->audioClip = audioClip;
}

void AudioComponent::OnStart(const AppContext& context)
{
	this->audioManagerRef = context.audioManager;
}

void AudioComponent::Update(const AppContext& context, float deltaTime)
{
	if (auto transform = GetOwner()->GetComponent<TransformComponent>()) {
		this->listenerPosition = transform->GetWorldPosition();
	}
	context.audioManager->SetListenerPosition(this->listenerPosition);
}

void AudioComponent::OnDestroy(const AppContext& context)
{
	StopPlaying();
}

void AudioComponent::PlayOnce()
{
	if(this->is3D)
	{
		this->audioManagerRef->PlaySoundAt(
			this->audioClip, 
			this->looping,
			this->audioPosition, 
			this->volume, 
			this->pitch);
	}
	else
	{
		this->audioManagerRef->PlaySoundEffect(
			this->audioClip, 
			this->looping,
			this->volume, 
			this->pitch);
	}
}

void AudioComponent::PlayInfinity()
{
	if (this->is3D)
	{
		this->audioManagerRef->PlaySoundAt(
			this->audioClip,
			this->looping,
			this->audioPosition,
			this->volume,
			this->pitch
		);
	}
	else
	{
		this->audioManagerRef->PlaySoundEffect(
			this->audioClip,
			this->looping,
			this->volume,
			this->pitch);
	}
}

bool AudioComponent::StopPlaying()
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	this->audioManagerRef->StopAudio(this->audioClip);
	return true;
}


float AudioComponent::GetVolume()
{
	return this->volume;
}

bool AudioComponent::SetVolume(float volume)
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	else {
		this->volume;
	}
	return true;
}


float AudioComponent::GetPitch()
{
	return this->pitch;
}

bool AudioComponent::SetPitch(float pitch)
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	else {
		this->pitch = pitch;
	}
	return true;
}


bool AudioComponent::IsAudio3D()
{
	return this->is3D;
}

bool AudioComponent::SetAudio3D(bool state)
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	else {
		this->is3D = state;
	}
	return true;
}


bool AudioComponent::IsLooping()
{
	return this->looping;
}

bool AudioComponent::SetLooping(bool state)
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	else {
		this->looping = state;
	}
	return true;
}


DirectX::XMFLOAT3 AudioComponent::GetAudioPosition()
{
	return this->audioPosition;
}

bool AudioComponent::SetAudioPoition(DirectX::XMFLOAT3 audioPoition)
{
	if (!this->audioClip)
	{
		LogWarn("AudioComponent dose not have audioClip!");
		return false;
	}
	else {
		this->audioPosition = audioPoition;
	}
	return true;
}


bool AudioComponent::ChangeAudioClip(std::shared_ptr<AudioClip> audioClip)
{
	return false;
}
