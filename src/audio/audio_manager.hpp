#pragma once

#include <xaudio2.h>
#include <x3daudio.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <DirectXMath.h>

class Application;
class AssetManager;
class AudioClip;

using SoundId = uint32_t;

struct ActiveSound
{
	SoundId id = 0;
	IXAudio2SourceVoice* voice = nullptr;
	std::shared_ptr<AudioClip> clip;
	bool is3D = false;
	DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
	float volume = 1.0f;
	float pitch = 1.0f;
	float minDistance = 0.0f;
	float maxDistance = 5.0f;
	bool looping = false;
};

class AudioManager
{
private:
	IXAudio2* xAudio2 = nullptr;
	IXAudio2MasteringVoice* masteringVoice = nullptr;
	IXAudio2SourceVoice* musicVoice = nullptr;
	std::shared_ptr<AudioClip> currentMusicClip = nullptr;

	X3DAUDIO_HANDLE x3dAudio = {};
	uint32_t destinationChannels = 0;

	Application *application = nullptr;
	AssetManager* assetManager = nullptr;

	DirectX::XMFLOAT3 listenerPosition{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 listenerOrientFront{ 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 listenerOrientTop{ 0.0f, 1.0f, 0.0f };

	float musicVolume = 1.0f;
	bool isMusicPaused = false;
	float pitchModifier = 1.0f;

	SoundId nextSoundId = 1;
	std::vector<ActiveSound> activeVoices;

	float GetResultingSoundVolume(float volume) const;
	float GetResultingMusicVolume(float volume) const;

public:
	AudioManager() = default;
	~AudioManager() = default;

	bool Initialize(AssetManager* assetManager, Application *application);
	void Shutdown();

	void FixedUpdate(float deltaTime);

	void SetListener(const DirectX::XMFLOAT3& listenerPosition, const DirectX::XMFLOAT3& front, const DirectX::XMFLOAT3& top);

	// SFX / 2D Playback
	SoundId PlaySoundEffect(std::shared_ptr<AudioClip> clip, bool loop, float volume = 1.0f, float pitch = 1.0f);

	// 3D Positional Playback
	SoundId PlaySoundAt(std::shared_ptr<AudioClip> clip, bool looping, DirectX::XMFLOAT3 position, float volume = 1.0f, float pitch = 1.0f, float minDistance = 0.0f, float maxDistance = 5.0f);
	void UpdateSoundPosition(SoundId id, const DirectX::XMFLOAT3& position);
	void UpdateAudio(SoundId id, float minDistance, float maxDistance, bool looping, bool is3D, float volume, float pitch = 1.0f);
	void UpdateAudio(SoundId id, float radius, bool looping, bool is3D, float volume);
	void StopSound(SoundId id);
	bool IsSoundPlaying(SoundId id) const;

	void StopAudio(std::shared_ptr<AudioClip> clip);

	// Pitch Modifier
	void SetPitchModifier(float pitch);
	float GetPitchModifier() const { return this->pitchModifier; }

	// Music Controls
	bool PlayMusic(std::shared_ptr<AudioClip> clip, bool loop = true, float volume = 1.0f, float pitch = 1.0f);
	void StopMusic();
	void PauseMusic();
	void ResumeMusic();
	bool IsMusicPlaying() const;
	bool IsMusicPaused() const { return this->isMusicPaused; }

	void SetMusicVolume(float volume);
	float GetMusicVolume() const { return this->musicVolume; }
};