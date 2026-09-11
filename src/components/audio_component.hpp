#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"
#include "components/transform_component.hpp"
#include <DirectXMath.h>

class AudioComponent : public Component
{
    RegisterComponent(AudioComponent)

    std::shared_ptr<AudioClip> audioClip;

    AudioManager* audioManagerRef = nullptr;

    DirectX::XMFLOAT3 audioPosition = { 0,0,0 };
    DirectX::XMFLOAT3 listenerPosition = { 0,0,0 };
    float volume = 1.0f;
    float pitch = 1.0f;
    bool is3D = true;
    bool isAudioClipChanged = false;
    bool looping = false;

public:
    AudioComponent(Entity* owner, bool isActive,std::shared_ptr<AudioClip> audioClip);

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override;

    void PlayOnce();
    void PlayInfinity();
    bool StopPlaying();
    
    float GetVolume();
    bool SetVolume(float volume);

    float GetPitch();
    bool SetPitch(float pitch);

    bool IsAudio3D();
    bool SetAudio3D(bool state);

    bool IsLooping();
    bool SetLooping(bool state);

    DirectX::XMFLOAT3 GetAudioPosition();
    bool SetAudioPoition(DirectX::XMFLOAT3 audioPoition);

    bool ChangeAudioClip(std::shared_ptr<AudioClip> audioClip);

};