#pragma once

#include "scene/component.hpp"
#include "assets/types/audio_clip.hpp"
#include <vector>
#include <memory>

class AudioManager;
class TransformComponent;

struct SoundEntry
{
    std::shared_ptr<AudioClip> clip;
    float minVolume = 0.8f;
    float maxVolume = 1.0f;
    float minPitch = 0.95f;
    float maxPitch = 1.05f;

    SoundEntry() = default;
    SoundEntry(std::shared_ptr<AudioClip> clip,
        float minVolume = 0.8f, float maxVolume = 1.0f,
        float minPitch = 0.95f, float maxPitch = 1.05f)
        : clip(clip), minVolume(minVolume), maxVolume(maxVolume), minPitch(minPitch), maxPitch(maxPitch) {
    }
};

struct SoundLayer
{
    std::vector<SoundEntry> sounds;

    void AddSound(std::shared_ptr<AudioClip> clip,
        float minVolume = 0.8f, float maxVolume = 1.0f,
        float minPitch = 0.95f, float maxPitch = 1.05f)
    {
        sounds.emplace_back(clip, minVolume, maxVolume, minPitch, maxPitch);
    }
	void AddSound(const SoundEntry& entry)
	{
		sounds.push_back(entry);
	}
};

class CharacterAudioComponent : public Component
{
    RegisterComponent(CharacterAudioComponent)

public:
    CharacterAudioComponent(Entity* owner, bool isActive = true);
    ~CharacterAudioComponent() override = default;

    void SetIsPlayer(bool player) { this->isPlayer = player; }
    bool IsPlayer() const { return this->isPlayer; }

    void Set3DDistance(float minDistance, float maxDistance) {
        this->minDistance = minDistance;
        this->maxDistance = maxDistance;
    }


    void AddLightAttackLayer(const SoundLayer& layer) { this->lightAttackLayers.push_back(layer); }
    void AddHeavyAttackLayer(const SoundLayer& layer) { this->heavyAttackLayers.push_back(layer); }

    void PlayLightAttack() { PlayLayers(this->lightAttackLayers); }
    void PlayHeavyAttack() { PlayLayers(this->heavyAttackLayers); }


    void AddFootstepLayer(const SoundLayer& layer) { this->footstepLayers.push_back(layer); }
    void SetFootstepInterval(float meters) { this->footstepIntervalMeters = meters; }
    void PlayFootstep() { PlayLayers(this->footstepLayers); }

    void AddMovementDistance(float distance)
    {
        this->movementDistanceAccumulator += distance;
        if (this->movementDistanceAccumulator >= this->footstepIntervalMeters)
        {
            this->movementDistanceAccumulator = 0.0f;
            PlayFootstep();
        }
    }

private:
    void OnStart(const AppContext& context) override;
    void PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch);
    void PlayLayers(const std::vector<SoundLayer>& layers);

    bool isPlayer = false;
    float minDistance = 1.0f;
    float maxDistance = 15.0f;

    std::vector<SoundLayer> lightAttackLayers;
    std::vector<SoundLayer> heavyAttackLayers;
    std::vector<SoundLayer> footstepLayers;

    float footstepIntervalMeters = 1.4f;
    float movementDistanceAccumulator = 0.0f;

    AudioManager* audioManager = nullptr;
    TransformComponent* transform = nullptr;
};