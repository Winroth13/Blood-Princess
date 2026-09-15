#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"
#include "components/transform_component.hpp"
#include <DirectXMath.h>

class AudioComponent : public Component
{
    RegisterComponent(AudioComponent)

    ExportProperty(std::shared_ptr<AudioClip>, audioClip, nullptr);

    AudioManager* audioManagerRef = nullptr;
    ExportProperty(DirectX::XMFLOAT3, audioPosition, { 0,0,0 });
    ExportPropertyRanged(float, volume, "", 0.0f, 5.0f, 0.01f, 1.0f);
    //ExportPropertyChanged(volume, );
    ExportPropertyRanged(float, pitch, "", 0.0f, 5.0f, 0.01f, 1.0f);
    ExportProperty(bool, is3D, true);
    bool isAudioClipChanged = false;
    ExportProperty(bool, looping, true);
    ExportProperty(int, currentSoundId, 0);
public:
    AudioComponent(Entity* owner, bool isActive,std::shared_ptr<AudioClip> audioClip);

    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void Render(const RenderView& view, RenderQueue& queue) override {};
    void OnDestroy(const AppContext& context) override;

    void Play();
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
    bool SetAudioPosition(DirectX::XMFLOAT3 audioPoition);

    bool ChangeAudioClip(std::shared_ptr<AudioClip> audioClip);

};