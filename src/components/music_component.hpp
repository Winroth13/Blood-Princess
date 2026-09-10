#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"

class MusicComponent : public Component 
{
	std::shared_ptr<AudioClip> audioClip;

	bool isAudioClipChanged = false;
	float volume = 1.0f;
	float pitch = 1.0f;
	bool looping = true;
	bool paused = false;
public:
	MusicComponent(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip);
	
	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override;
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;





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