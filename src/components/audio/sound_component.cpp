#include "sound_component.hpp"
#include "scene/entity.hpp"
#include "assets/types/sound.hpp"
#include "core/rng.hpp"
#include "components/core/transform_component.hpp"
#include "audio/audio_manager.hpp"
#include "assets/types/audio_clip.hpp"
#include "core/logging.hpp"

void SoundComponent::OnDestroy(const AppContext &context)
{
    this->StopPlaying();
}

void SoundComponent::OnStart(const AppContext &context)
{
	this->audioManager = context.audioManager;
    if (this->autoPlay)
        this->Play();
}

void SoundComponent::Update(const AppContext &context, float deltaTime)
{
    if (!this->is3d)
        return;

    if (auto transform = GetOwner()->GetComponent<TransformComponent>()) {
        if (this->currentSoundId != 0 && this->audioManager) {
            this->audioManager->UpdateSoundPosition(this->currentSoundId, transform->GetWorldPosition());
            this->audioManager->UpdateAudio(
                this->currentSoundId, 
                this->minDistance, 
                this->maxDistance, 
                false, 
                this->is3d, 
                this->volume
            );
        }
    }
}

void SoundComponent::PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch)
{
    if (!this->audioManager)
        return;

    TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>();
    if (transform != nullptr && this->is3d)
    {
        this->currentSoundId = this->audioManager->PlaySoundAt(
            clip,
            false,
            transform->GetWorldPosition(),
            volume * this->volume,
            pitch,
            this->minDistance,
            this->maxDistance
        );
    }
    else
    {
        this->currentSoundId = this->audioManager->PlaySoundEffect(
            clip, 
            false, 
            volume * this->volume, 
            pitch
        );
    }
}

bool SoundComponent::StopPlaying()
{
    if (this->sound == nullptr)
    {
        LogWarn("No sound is assigned.");
        return false;
    }

    if (this->sound->GetLayerCount() == 0)
    {
        LogWarn("Cannot to stop sound with no layers: %s", this->sound->GetPath().c_str());
        return false;
    }

    if (this->currentSoundId != 0 && this->audioManager) {
        this->audioManager->StopSound(this->currentSoundId);
        this->currentSoundId = 0;
    }
    return true;
}

void SoundComponent::Play(float volume, float pitch)
{
	if (this->sound == nullptr)
	{
        LogWarn("No sound is assigned.");
		return;
	}

    if (this->sound->GetLayerCount() == 0)
    {
        LogWarn("Cannot to play sound with no layers: %s", this->sound->GetPath().c_str());
        return;
    }

	int index = 0;
    do
    {
        index = RNG::Int(0, static_cast<int>(this->sound->GetLayerCount() - 1));
    } while (index == this->lastLayerIndex);
	
	auto &layer = this->sound->GetLayer(static_cast<size_t>(index));
    
    this->PlayClip(
        layer.audioClip,
        RNG::Float(layer.volumeRange.x, layer.volumeRange.y),
        RNG::Float(layer.pitchRange.x, layer.pitchRange.y) * pitch
    );
}
