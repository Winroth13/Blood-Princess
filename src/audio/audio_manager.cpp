#include "audio_manager.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/audio_clip.hpp"
#include "core/logging.hpp"

// Shared attenuation curve used for distance-based volume calculations
static X3DAUDIO_DISTANCE_CURVE_POINT g_VolumeCurvePoints[2] = {
	{ 0.0f, 1.0f },
	{ 1.0f, 0.0f }
};
static X3DAUDIO_DISTANCE_CURVE g_VolumeCurve = {
	&g_VolumeCurvePoints[0],
	2
};

void AudioManager::Update(float deltaTime)
{
	for (size_t i = 0; i < this->activeVoices.size(); )
	{
		XAUDIO2_VOICE_STATE state;
		this->activeVoices[i].voice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			this->activeVoices[i].voice->DestroyVoice();
			this->activeVoices.erase(this->activeVoices.begin() + i);
		}
		else
		{
			++i;
		}
	}

	X3DAUDIO_LISTENER listener = {};
	listener.OrientFront = { this->listenerOrientFront.x, this->listenerOrientFront.y, this->listenerOrientFront.z };
	listener.OrientTop = { this->listenerOrientTop.x, this->listenerOrientTop.y, this->listenerOrientTop.z };
	listener.Position = { this->listenerPosition.x, this->listenerPosition.y, this->listenerPosition.z };


	for (auto& active : this->activeVoices)
	{
		if (!active.is3D || !active.voice) continue;


		X3DAUDIO_EMITTER emitter = {};
		emitter.OrientFront = { 0.0f, 0.0f, 1.0f };
		emitter.OrientTop = { 0.0f, 1.0f, 0.0f };
		emitter.Position = active.position;
		emitter.ChannelCount = active.clip->GetChannels();
		emitter.CurveDistanceScaler = (active.radius > 0.0f) ? active.radius : 1.0f;
		emitter.DopplerScaler = 1.0f;
		emitter.pVolumeCurve = &g_VolumeCurve;

		std::vector<float> channelAzimuths;
		if (emitter.ChannelCount > 1)
		{
			channelAzimuths.resize(emitter.ChannelCount, 0.0f);
			if (emitter.ChannelCount == 2)
			{
				channelAzimuths[0] = -X3DAUDIO_PI / 4.0f;
				channelAzimuths[1] = X3DAUDIO_PI / 4.0f;
			}
			emitter.pChannelAzimuths = channelAzimuths.data();
			emitter.ChannelRadius = active.radius;
		}
		else
		{
			emitter.pChannelAzimuths = nullptr;
			emitter.ChannelRadius = 0.0f;
		}

		std::vector<float> matrix(emitter.ChannelCount * this->destinationChannels, 0.0f);
		X3DAUDIO_DSP_SETTINGS dspSettings = {};
		dspSettings.SrcChannelCount = emitter.ChannelCount;
		dspSettings.DstChannelCount = this->destinationChannels;
		dspSettings.pMatrixCoefficients = matrix.data();

		X3DAudioCalculate(this->x3dAudio, &listener, &emitter, X3DAUDIO_CALCULATE_MATRIX, &dspSettings);

		for (float& coeff : matrix) {
			coeff *= active.volume;
		}

		active.voice->SetOutputMatrix(nullptr, emitter.ChannelCount, this->destinationChannels, matrix.data());
	}
}

void AudioManager::UpdateSoundPosition(SoundId id, const DirectX::XMFLOAT3& newPosition)
{
	if (id == 0) return;
	for (auto& active : this->activeVoices)
	{
		if (active.id == id)
		{
			active.position = newPosition;
			break;
		}
	}
}

void AudioManager::UpdateAudio(SoundId id, float radius, bool looping, bool is3D, float volume)
{
	for (auto& active : this->activeVoices)
	{
		if (active.id == id) {

			if (active.looping != looping && active.voice && active.clip) {
				active.voice->Stop(0);
				active.voice->FlushSourceBuffers();

				XAUDIO2_BUFFER buffer = {};
				buffer.AudioBytes = active.clip->GetAudioSize();
				buffer.pAudioData = active.clip->GetAudioData().data();
				buffer.Flags = XAUDIO2_END_OF_STREAM;

				if (looping)
				{
					buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
				}
				else
				{
					buffer.LoopCount = 0;
				}
				HRESULT hr = active.voice->SubmitSourceBuffer(&buffer);
				if (SUCCEEDED(hr))
				{
					active.voice->Start(0);
				}
				else
				{
					LogError("Failed to re-submit source buffer during loop toggle! HRESULT: 0x%08X", hr);
				}

			}
			active.radius = radius;
			active.is3D = is3D;
			active.volume = volume;
			active.looping = looping;
			break;
		}
		
	}
}

void AudioManager::StopSound(SoundId id)
{
	if (id == 0) return;
	for (auto it = this->activeVoices.begin(); it != this->activeVoices.end(); ++it)
	{
		if (it->id == id)
		{
			if (it->voice)
			{
				it->voice->Stop(0);
				it->voice->DestroyVoice();
			}
			this->activeVoices.erase(it);
			break;
		}
	}
}

bool AudioManager::IsSoundPlaying(SoundId id) const
{
	if (id == 0) return false;
	for (const auto& active : this->activeVoices)
	{
		if (active.id == id)
		{
			return true;
		}
	}
	return false;
}

bool AudioManager::Initialize(AssetManager* assetManager)
{
	LogInfo("Creating Audio Manager...");
	LogIndent();

	this->assetManager = assetManager;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
	{
		LogError("Failed to initialize COM! HRESULT: 0x%08X", hr);
		LogUnindent();
		return false;
	}

	hr = XAudio2Create(&this->xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		LogError("Failed to create XAudio2 engine! HRESULT: 0x%08X", hr);
		LogUnindent();
		return false;
	}

	hr = this->xAudio2->CreateMasteringVoice(&this->masteringVoice);
	if (FAILED(hr))
	{
		LogError("Failed to create Mastering Voice! HRESULT: 0x%08X", hr);
		LogUnindent();
		return false;
	}

	DWORD channelMask = 0;
	hr = this->masteringVoice->GetChannelMask(&channelMask);
	if (FAILED(hr))
	{
		LogError("Failed to get Channel Mask! HRESULT: 0x%08X", hr);
		LogUnindent();
		return false;
	}

	XAUDIO2_VOICE_DETAILS details;
	this->masteringVoice->GetVoiceDetails(&details);
	this->destinationChannels = details.InputChannels;

	X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, this->x3dAudio);

	LogInfo("Audio Manager initialized with %u destination channels", this->destinationChannels);
	LogUnindent();
	return true;
}

void AudioManager::Shutdown()
{
	LogInfo("Destroying Audio Manager...");
	LogIndent();

	this->StopMusic();

	for (ActiveSound& active : this->activeVoices)
	{
		if (active.voice)
		{
			active.voice->Stop(0);
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}
	this->activeVoices.clear();

	if (this->masteringVoice)
	{
		this->masteringVoice->DestroyVoice();
		this->masteringVoice = nullptr;
	}

	if (this->xAudio2)
	{
		this->xAudio2->Release();
		this->xAudio2 = nullptr;
	}

	CoUninitialize();
	LogUnindent();
}

void AudioManager::SetListener(const DirectX::XMFLOAT3& listenerPosition, const DirectX::XMFLOAT3& front, const DirectX::XMFLOAT3& top)
{
	this->listenerPosition = listenerPosition;
	this->listenerOrientFront = front;
	this->listenerOrientTop = top;
}

SoundId AudioManager::PlaySoundEffect(std::shared_ptr<AudioClip> clip, bool loop, float volume, float pitch)
{
	if (!clip)
	{
		LogError("AudioClip is nullptr!");
		return 0;
	}

	if (!this->xAudio2)
	{
		LogError("Audio Manager is not initialized!");
		return 0;
	}

	// Clean up finished voices
	for (size_t i = 0; i < this->activeVoices.size(); )
	{
		XAUDIO2_VOICE_STATE state;
		this->activeVoices[i].voice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			this->activeVoices[i].voice->DestroyVoice();
			this->activeVoices.erase(this->activeVoices.begin() + i);
		}
		else
		{
			++i;
		}
	}
	float maxFrequencyRatio = 4.0f;

	if (pitch > maxFrequencyRatio) {
		maxFrequencyRatio = pitch;
	}
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT hr = this->xAudio2->CreateSourceVoice(
		&sourceVoice,
		&clip->GetWaveFormat(),
		0,
		maxFrequencyRatio

	);
	if (FAILED(hr))
	{
		LogError("Failed to create Source Voice! HRESULT: 0x%08X", hr);
		return 0;
	}
	sourceVoice->SetFrequencyRatio(pitch);
	sourceVoice->SetVolume(volume);

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = clip->GetAudioSize();
	buffer.pAudioData = clip->GetAudioData().data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (loop)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	hr = sourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
	{
		LogError("Failed to submit source buffer! HRESULT: 0x%08X", hr);
		sourceVoice->DestroyVoice();
		return 0;
	}

	hr = sourceVoice->Start(0);
	if (FAILED(hr))
	{
		LogError("Failed to start Source Voice! HRESULT: 0x%08X", hr);
		sourceVoice->DestroyVoice();
		return 0;
	}

	SoundId id = this->nextSoundId++;
	this->activeVoices.push_back({ id, sourceVoice, clip, false, { 0.0f, 0.0f, 0.0f }, volume});
	return id;
}


SoundId AudioManager::PlaySoundAt(std::shared_ptr<AudioClip> clip, bool looping, DirectX::XMFLOAT3 position, float volume, float pitch, float radius)
{
	if (!clip)
	{
		LogError("AudioClip is nullptr!");
		return 0;
	}

	if (!this->xAudio2)
	{
		LogError("Audio Manager is not initialized!");
		return 0;
	}

	for (size_t i = 0; i < this->activeVoices.size(); )
	{
		XAUDIO2_VOICE_STATE state;
		this->activeVoices[i].voice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			this->activeVoices[i].voice->DestroyVoice();
			this->activeVoices.erase(this->activeVoices.begin() + i);
		}
		else
		{
			++i;
		}
	}
	float maxFrequencyRatio = 4.0f;
	if (pitch > maxFrequencyRatio) {
		maxFrequencyRatio = pitch;
	}

	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT hr = this->xAudio2->CreateSourceVoice(
		&sourceVoice, 
		&clip->GetWaveFormat(),
		0,
		maxFrequencyRatio
	);
	if (FAILED(hr))
	{
		LogError("Failed to create Source Voice! HRESULT: 0x%08X", hr);
		return 0;
	}
	sourceVoice->SetFrequencyRatio(pitch);

	// Calculate 3D positional audio
	X3DAUDIO_LISTENER listener = {};
	listener.OrientFront = { this->listenerOrientFront.x, this->listenerOrientFront.y, this->listenerOrientFront.z };
	listener.OrientTop = { this->listenerOrientTop.x, this->listenerOrientTop.y, this->listenerOrientTop.z };
	listener.Position = { this->listenerPosition.x, this->listenerPosition.y, this->listenerPosition.z };

	X3DAUDIO_EMITTER emitter = {};
	emitter.OrientFront = { 0.0f, 0.0f, 1.0f };
	emitter.OrientTop = { 0.0f, 1.0f, 0.0f };
	emitter.Position = { position.x, position.y, position.z };
	emitter.ChannelCount = clip->GetChannels();
	emitter.CurveDistanceScaler = (radius > 0.0f) ? radius : 1.0f;
	emitter.DopplerScaler = 1.0f;

	emitter.pVolumeCurve = &g_VolumeCurve;
	std::vector<float> channelAzimuths;
	if (emitter.ChannelCount > 1)
	{
		channelAzimuths.resize(emitter.ChannelCount, 0.0f);
		if (emitter.ChannelCount == 2)
		{
			channelAzimuths[0] = -X3DAUDIO_PI / 4.0f;
			channelAzimuths[1] = X3DAUDIO_PI / 4.0f;
		}
		emitter.pChannelAzimuths = channelAzimuths.data();
		emitter.ChannelRadius = (radius > 0.0f) ? radius : 20.0f;;
	}
	else
	{
		emitter.pChannelAzimuths = nullptr;
		emitter.ChannelRadius = 0.0f;
	}

	std::vector<float> matrix(emitter.ChannelCount * this->destinationChannels, 0.0f);

	X3DAUDIO_DSP_SETTINGS dspSettings = {};
	dspSettings.SrcChannelCount = emitter.ChannelCount;
	dspSettings.DstChannelCount = this->destinationChannels;
	dspSettings.pMatrixCoefficients = matrix.data();

	X3DAudioCalculate(this->x3dAudio, &listener, &emitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dspSettings);

	for (float& coeff : matrix)
	{
		coeff *= volume;
	}

	sourceVoice->SetOutputMatrix(nullptr, emitter.ChannelCount, this->destinationChannels, matrix.data());

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = clip->GetAudioSize();
	buffer.pAudioData = clip->GetAudioData().data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	if (looping)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	hr = sourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
	{
		LogError("Failed to submit source buffer! HRESULT: 0x%08X", hr);
		sourceVoice->DestroyVoice();
		return 0;
	}

	hr = sourceVoice->Start(0);
	if (FAILED(hr))
	{
		LogError("Failed to start Source Voice! HRESULT: 0x%08X", hr);
		sourceVoice->DestroyVoice();
		return 0;
	}

	SoundId id = this->nextSoundId++;
	this->activeVoices.push_back({ id, sourceVoice, clip, true, position, volume , radius});
	return id;
}

void AudioManager::StopAudio(std::shared_ptr<AudioClip> clip)
{
	if (!clip) return;

	for (auto it = this->activeVoices.begin() ; it != this->activeVoices.end();)
	{
		if (it->clip == clip)
		{
			if (it->voice)
			{
				it->voice->Stop(0);
				it->voice->DestroyVoice();
			}
			it = this->activeVoices.erase(it);
		}
		else
		{
			++it;
		}
	}
}


bool AudioManager::PlayMusic(std::shared_ptr<AudioClip> clip, bool loop, float volume, float pitch)
{
	if (!clip)
	{
		LogError("AudioClip is nullptr for music!");
		return false;
	}

	if (!this->xAudio2)
	{
		LogError("Audio Manager is not initialized!");
		return false;
	}

	// Stop previous music if playing
	this->StopMusic();
	float maxFrequencyRatio = 4.0f;

	if (pitch > maxFrequencyRatio) {
		maxFrequencyRatio = pitch;
	}

	HRESULT hr = this->xAudio2->CreateSourceVoice(
		&this->musicVoice, 
		&clip->GetWaveFormat(),
		0,
		maxFrequencyRatio
	);

	if (FAILED(hr))
	{
		LogError("Failed to create Music Source Voice! HRESULT: 0x%08X", hr);
		return false;
	}

	this->musicVoice->SetVolume(volume * this->musicVolume);
	this->musicVoice->SetFrequencyRatio(pitch);

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = clip->GetAudioSize();
	buffer.pAudioData = clip->GetAudioData().data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (loop)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}


	hr = this->musicVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
	{
		LogError("Failed to submit music source buffer! HRESULT: 0x%08X", hr);
		this->musicVoice->DestroyVoice();
		this->musicVoice = nullptr;
		return false;
	}

	hr = this->musicVoice->Start(0);
	if (FAILED(hr))
	{
		LogError("Failed to start Music Source Voice! HRESULT: 0x%08X", hr);
		this->musicVoice->DestroyVoice();
		this->musicVoice = nullptr;
		return false;
	}

	this->currentMusicClip = clip;
	this->isMusicPaused = false;
	return true;
}

void AudioManager::StopMusic()
{
	if (this->musicVoice)
	{
		this->musicVoice->Stop(0);
		this->musicVoice->FlushSourceBuffers();
		this->musicVoice->DestroyVoice();
		this->musicVoice = nullptr;
	}

	this->currentMusicClip = nullptr;
	this->isMusicPaused = false;
}

void AudioManager::PauseMusic()
{
	if (this->musicVoice && !this->isMusicPaused)
	{
		this->musicVoice->Stop(0);
		this->isMusicPaused = true;
	}
}

void AudioManager::ResumeMusic()
{
	if (this->musicVoice && this->isMusicPaused)
	{
		this->musicVoice->Start(0);
		this->isMusicPaused = false;
	}
}

bool AudioManager::IsMusicPlaying() const
{
	if (!this->musicVoice || this->isMusicPaused)
	{
		return false;
	}

	XAUDIO2_VOICE_STATE state;
	this->musicVoice->GetState(&state);
	return state.BuffersQueued > 0;
}

void AudioManager::SetMusicVolume(float volume)
{
	this->musicVolume = volume;
	if (this->musicVoice)
	{
		this->musicVoice->SetVolume(volume);
	}
}