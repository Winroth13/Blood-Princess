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
    void AddLightAttackSwingLayer(const SoundLayer& layer) { this->lightAttackSwingLayers.push_back(layer); }
    void AddHeavyAttackLayer(const SoundLayer& layer) { this->heavyAttackLayers.push_back(layer); }
    void AddHeavyAttackSwingLayer(const SoundLayer& layer) { this->heavyAttackSwingLayers.push_back(layer); }
    void AddGoblinKnockbackLayer(const SoundLayer& layer) { this->goblinKnockbackLayers.push_back(layer); }
    void AddAttackShoutLayer(const SoundLayer &layer) { this->attackShoutLayer.push_back(layer); }
    void AddHeavyAttackShoutLayer(const SoundLayer &layer) { this->heavyAttackShoutLayer.push_back(layer); }

    void PlayLightAttackHit() { PlayLayers(this->lightAttackLayers); }
	void PlayLightAttackSwing() { PlayLayers(this->lightAttackSwingLayers); }
    void PlayHeavyAttackHit() { PlayLayers(this->heavyAttackLayers); }
	void PlayHeavyAttackSwing() { PlayLayers(this->heavyAttackSwingLayers); }
    void PlayGoblinKnockback() { PlayLayers(this->goblinKnockbackLayers); }
    void PlayAttackShout() { PlayLayers(this->attackShoutLayer); }
    void PlayHeavyAttackShout() { PlayLayers(this->heavyAttackShoutLayer); }

    void AddFootstepLayer(const SoundLayer& layer) { this->footstepLayers.push_back(layer); }
    void SetFootstepInterval(float meters) { this->footstepIntervalMeters = meters; }
    void PlayFootstep() { PlayLayers(this->footstepLayers); }

    void AddMovementDistance(float distance);

private:
    void OnStart(const AppContext& context) override;
    void PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch);

    void PlayLayers(const std::vector<SoundLayer>& layers);

    bool isPlayer = false;
    float minDistance = 1.0f;
    float maxDistance = 15.0f;

    std::vector<SoundLayer> lightAttackLayers;
    std::vector<SoundLayer> lightAttackSwingLayers;

    std::vector<SoundLayer> heavyAttackLayers;
    std::vector<SoundLayer> heavyAttackSwingLayers;

    std::vector<SoundLayer> footstepLayers;

    std::vector<SoundLayer> goblinKnockbackLayers;

    std::vector<SoundLayer> attackShoutLayer;
    std::vector<SoundLayer> heavyAttackShoutLayer;

    float footstepIntervalMeters = 3.0f;
    float movementDistanceAccumulator = 0.0f;

    AudioManager* audioManager = nullptr;
    TransformComponent* transform = nullptr;
};