#pragma once
#include "assets/asset.hpp"
#include <memory>
#include <string>
#include <xaudio2.h>
#include <vector>

class AudioClip : public Asset
{
	WAVEFORMATEX waveFormat = {};
	std::vector<uint8_t> pcmData = {};

	uint32_t channels;
	uint32_t sampleRate;

	// TODO LOOK UP IF THERE IS REALEASE
	XAUDIO2_BUFFER* audioBuffer;
public:
	AudioClip() = default;
	~AudioClip() = default;

	bool Create(
		const short *samples,
		const uint32_t totalSampels,
		const uint32_t channels,
		const uint32_t sampleRate
	);

	const WAVEFORMATEX& GetWaveFormat() const { return this->waveFormat; }
	const std::vector<uint8_t>& GetAudioData() const { return this->pcmData; }
	uint32_t GetAudioSize() const { return static_cast<uint32_t>(this->pcmData.size()); }
	uint32_t GetChannels() const { return this->channels; }
	uint32_t GetSampleRate() const { return this->sampleRate; }
	XAUDIO2_BUFFER* GetAudioBuffer() { return this->audioBuffer; }

	bool CreateBuffer(bool loop = false);

};