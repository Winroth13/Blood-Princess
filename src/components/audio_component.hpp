#pragma once
#include "scene/component.hpp"
#include "audio/audio_manager.hpp"
#include "components/transform_component.hpp"
#include <DirectXMath.h>
#include <algorithm>
class AudioComponent : public Component
{
    RegisterComponent(AudioComponent)

    ExportProperty(std::shared_ptr<AudioClip>, audioClip, nullptr);

    AudioManager* audioManagerRef = nullptr;
    ExportPropertyRanged(float, volume, "", 0.0f, 5.0f, 0.01f, 1.0f);
    ExportPropertyRanged(float, pitch, "", 0.0f, 5.0f, 0.01f, 1.0f);
    ExportProperty(bool, is3D, true);
    bool isAudioClipChanged = false;
    ExportProperty(bool, looping, true);

    ExportPropertyRanged(float, minDistance, "m", 0.0f, 1.0f, 0.01f, 0.0f);
    ExportPropertyRanged(float, maxDistance, "m", 0.0f, 10.0f, 0.01f, 5.0f);
    int currentSoundId = 0;
    bool playOnStart = false;

public:
    AudioComponent(Entity* owner, bool isActive,std::shared_ptr<AudioClip> audioClip);

private:
    void OnStart(const AppContext& context) override;
    void Update(const AppContext& context, float deltaTime) override;
    void OnDestroy(const AppContext& context) override;

public:
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

    float GetMinDistance();
    bool SetMinDistance(float minDistance = 0.0f);

    float GetMaxDistance();
    bool SetMaxDistance(float maxDistance = 5.0f);

    float GetAudioRadius();
    bool SetAudioRadius(float radius = 5.0f);

    bool ChangeAudioClip(std::shared_ptr<AudioClip> audioClip);
    void SetPlayOnStart(bool play);

};