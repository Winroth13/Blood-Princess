#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"

class Music : public Component 
{

	std::shared_ptr<AudioClip> audioClip;
	float volume = 1.0f;
	float pitch = 1.0f;
	bool looping = true;

public:
	Music(Entity* owner, bool isActive, std::shared_ptr<AudioClip> audioClip);

	void OnStart(const AppContext& context) override;
	void Update(const AppContext& context, float deltaTime) override {};
	void Render(const RenderView& view, RenderQueue& queue) override {};
	void OnDestroy(const AppContext& context) override;

};