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
	int currentSoundId = 0;

	bool is3d = true;
	bool autoPlay = false;

public:
	SoundComponent(Entity *owner, bool isActive = true) : Component(owner, isActive) { WantsUpdate(); }
	~SoundComponent() override = default;
	void OnDestroy(const AppContext &context) override;

	void Play(float volume = 1.0f);

	void SetSound(std::shared_ptr<Sound> sound) { this->sound = sound; }

	void SetIs3d(bool is3d) { this->is3d = is3d; }
	void SetAutoPlay(bool autoPlay) { this->autoPlay = autoPlay; }
private:
	void OnStart(const AppContext &context) override;

	void PlayClip(std::shared_ptr<AudioClip> clip, float volume, float pitch);
	bool StopPlaying();
};