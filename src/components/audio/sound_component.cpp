#include "sound_component.hpp"
#include "scene/entity.hpp"
#include "assets/types/sound.hpp"
#include "core/rng.hpp"
#include "components/core/transform_component.hpp"
#include "audio/audio_manager.hpp"
#include "assets/types/audio_clip.hpp"
#include "core/logging.hpp"

void SoundComponent::OnStart(const AppContext &context)
{
	this->audioManager = context.audioManager;
}

void SoundComponent::PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch)
{
    if (!this->audioManager)
        return;

    if (TransformComponent *transform = this->GetOwner()->GetComponent<TransformComponent>())
    {
        this->audioManager->PlaySoundAt(
            clip,
            false,
            transform->GetWorldPosition(),
            volume,
            pitch,
            this->minDistance,
            this->maxDistance
        );
    }
    else
    {
        this->audioManager->PlaySoundEffect(
            clip, 
            false, 
            volume, 
            pitch
        );
    }
}

void SoundComponent::Play(float volume)
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
        RNG::Float(layer.pitchRange.x, layer.pitchRange.y)
    );
}
