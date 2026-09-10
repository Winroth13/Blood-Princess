#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"


class AudioComponent : public Component
{

    std::string name;
    // pitch
    // looping
    // volume
    // is3D
    // music?
    // source file :)
    // autoStart

public:
    AudioComponent(Entity* owner, bool isActive,std::shared_ptr<AudioClip> audioClip);

    void OnStart(const AppContext& context) override;
    // Play sound
    void Update(const AppContext& context, float deltaTime) override;
    // If 3D, move sound position to transform component position
    void Render(const RenderView& view, RenderQueue& queue) override;
    // nada
    void OnDestroy(const AppContext& context) override;
    // Stop playing sound
};