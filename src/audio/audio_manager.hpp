#pragma once

#include <xaudio2.h>
#include <x3daudio.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <DirectXMath.h>

class AssetManager;
class AudioClip;

struct ActiveSound
{
	IXAudio2SourceVoice* voice = nullptr;
	std::shared_ptr<AudioClip> clip;
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

	AssetManager* assetManager = nullptr;

	DirectX::XMFLOAT3 listenerPosition{ 0,0,0 };

	float musicVolume = 1.0f;
	bool isMusicPaused = false;

	std::vector<ActiveSound> activeVoices;

public:
	AudioManager() = default;
	~AudioManager() = default;

	bool Initialize(AssetManager* assetManager);
	void Shutdown();

	void SetListenerPosition(DirectX::XMFLOAT3 listenerPosition);

	//std::shared_ptr<AudioClip> LoadSound(const std::string& filename);

	// SFX / 2D Playback
	//bool PlaySoundEffect(const std::string& filename, float volume = 1.0f, float pitch = 1.0f);
	bool PlaySoundEffect(std::shared_ptr<AudioClip> clip, bool loop, float volume = 1.0f, float pitch = 1.0f);

	// 3D Positional Playback
	//bool PlaySoundAt(const std::string& filename, DirectX::XMFLOAT3 position, float volume = 1.0f, float pitch = 1.0f);
	bool PlaySoundAt(std::shared_ptr<AudioClip> clip, bool looping, DirectX::XMFLOAT3 position, float volume = 1.0f, float pitch = 1.0f);
	bool StopSoundAt(std::shared_ptr<AudioClip> clip);

	void StopAudio(std::shared_ptr<AudioClip> clip);
	// Music Controls
	//bool PlayMusic(const std::string& filename, bool loop = true, float volume = 1.0f, float pitch = 1.0f);
	bool PlayMusic(std::shared_ptr<AudioClip> clip, bool loop = true, float volume = 1.0f, float pitch = 1.0f);
	void StopMusic();
	void PauseMusic();
	void ResumeMusic();
	bool IsMusicPlaying() const;
	bool IsMusicPaused() const { return this->isMusicPaused; }

	void SetMusicVolume(float volume);
	float GetMusicVolume() const { return this->musicVolume; }
};