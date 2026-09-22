#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"

class MusicComponent : public Component 
{
	RegisterComponent(MusicComponent);
	ExportProperty(std::shared_ptr<AudioClip>, audioClip, nullptr);
	bool isAudioClipChanged = false;
	ExportPropertyRanged(float, volume, "", 0.0f, 100.0f, 0.01f, 1.0f);
	ExportPropertyRanged(float, pitch, "", 0.0f, 5.0f, 0.01f, 1.0f);
	ExportProperty(bool, looping, true);
	ExportProperty(bool, paused, false);

	AudioManager* audioManagerRef = nullptr;

public:
	MusicComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip);
	
private:
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void LateUpdate(const AppContext &context, float deltaTime) override {}
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;

public:
	float GetVolume();
	bool SetVolume(float volume);

	float GetPitch();
	bool SetPitch(float pitch);

	bool GetLooping();
	bool SetLooping(bool looping);

	bool IsPaused();
	bool SetPaused(bool isPlaying);

	bool ChangeAudioClip(std::shared_ptr<AudioClip> audioClip);

};