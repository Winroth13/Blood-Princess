#include "audio_clip.hpp"
#include "core/logging.hpp"

//AudioClip::~AudioClip()
//{
//	LogInfo("Destroying AudioClip...");
//	LogIndent();
//	if (this->AudioBuffer) {
//		AudioBuffer)
//	}
//	LogUnindent();
//}

bool AudioClip::Create(
	const short* samples,
	uint32_t totalSamples,
	uint32_t channels,
	uint32_t sampleRate
)
{
	if (!samples || totalSamples == 0 || channels == 0 || sampleRate == 0)
	{
		LogError("Failed to create AudioClip: Invalid audio parameters");
		return false;
	}

	this->channels = channels;
	this->sampleRate = sampleRate;

	uint32_t byteSize = totalSamples * sizeof(short);
	this->pcmData.resize(byteSize);
	memcpy(this->pcmData.data(), samples, byteSize);

	this->waveFormat = {};
	this->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	this->waveFormat.nChannels = static_cast<WORD>(channels);
	this->waveFormat.nSamplesPerSec = sampleRate;
	this->waveFormat.wBitsPerSample = 16;
	this->waveFormat.nBlockAlign = (this->waveFormat.nChannels * this->waveFormat.wBitsPerSample) / 8;
	this->waveFormat.nAvgBytesPerSec = this->waveFormat.nSamplesPerSec * this->waveFormat.nBlockAlign;
	this->waveFormat.cbSize = 0;


	return true;
}

bool AudioClip::CreateBuffer(bool loop)
{
	
	audioBuffer->AudioBytes = this->GetAudioSize();
	audioBuffer->pAudioData = this->GetAudioData().data();

	audioBuffer->Flags = XAUDIO2_END_OF_STREAM;

	if (loop) {
		audioBuffer->LoopCount = XAUDIO2_LOOP_INFINITE;
		audioBuffer->LoopBegin = 0;
		audioBuffer->LoopLength = 0;
	}
	else
	{
		audioBuffer->LoopCount = 0;
	}

	audioBuffer->PlayBegin = 0;
	audioBuffer->PlayLength = 0;
	audioBuffer->pContext = nullptr;


	return true;
}

