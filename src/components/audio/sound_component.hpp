#pragma once

#include "scene/component.hpp"

class Sound;
class AudioClip;

class SoundComponent : public Component
{
	RegisterComponent(SoundComponent)

	ExportProperty(std::shared_ptr<Sound>, sound, nullptr);
	ExportPropertyRanged(float, minDistance, "m", 0.0f, 0.0f, 0.1f, 1.0f);
	ExportPropertyRanged(float, maxDistance, "m", 0.0f, 0.0f, 0.1f, 15.0f);

	AudioManager *audioManager = nullptr;

	int lastLayerIndex = -1;

public:
	SoundComponent(Entity *owner, bool isActive = true) : Component(owner, isActive) { WantsUpdate(); }
	~SoundComponent() override = default;

	void Play(float volume = 1.0f);

	void SetSound(std::shared_ptr<Sound> sound) { this->sound = sound; }

private:
	void OnStart(const AppContext &context) override;

	void PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch);
};