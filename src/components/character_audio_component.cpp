#include "character_audio_component.hpp"
#include "components/transform_component.hpp"
#include "audio/audio_manager.hpp"
#include "core/rng.hpp"
#include "scene/entity.hpp"

CharacterAudioComponent::CharacterAudioComponent(Entity* owner, bool isActive)
    : Component(owner, isActive)
{
    NoLateUpdate();
    NoRender();
}

void CharacterAudioComponent::OnStart(const AppContext& context)
{
    this->audioManager = context.audioManager;
    this->transform = this->GetOwner()->GetComponent<TransformComponent>();
}

void CharacterAudioComponent::PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch)
{
    if (!clip || !this->audioManager)
        return;

    if (this->isPlayer)
    {
        this->audioManager->PlaySoundEffect(clip, false, volume, pitch);
    }
    else if (this->transform)
    {
        this->audioManager->PlaySoundAt(
            clip,
            false,
            this->transform->GetWorldPosition(),
            volume,
            pitch,
            this->minDistance,
            this->maxDistance
        );
    }
}

void CharacterAudioComponent::PlayLayers(const std::vector<SoundLayer>& layers)
{
    for (const auto& layer : layers)
    {
        if (layer.sounds.empty())
            continue;

        int index = RNG::Int(0, static_cast<int>(layer.sounds.size()) - 1);
        const SoundEntry& entry = layer.sounds[index];

        if (!entry.clip)
            continue;

        float volume = (entry.minVolume >= entry.maxVolume)
            ? entry.minVolume
            : RNG::Float(entry.minVolume, entry.maxVolume);

        float pitch = (entry.minPitch >= entry.maxPitch)
            ? entry.minPitch
            : RNG::Float(entry.minPitch, entry.maxPitch);

        PlayClip(entry.clip, volume, pitch);
    }
}
