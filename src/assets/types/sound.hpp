#pragma once
#include "assets/asset.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <memory>
#include <vector>

/*
* 
* Example how the YAML file should look like:
* 
* attack_shout.sound
* 
* sound:
	- file: assets/audio/sfx/LightAttack/slash_attack_1.ogg
		min_volume: 1.0
		max_volume: 1.0
		min_pitch: 1.0
		max_pitch: 1.0

	- file: assets/audio/sfx/LightAttack/slash_attack_2.ogg
		min_volume: 1.0
		max_volume: 1.0
		min_pitch: 1.0
		max_pitch: 1.0
*/

class AudioClip;

class Sound : public Asset
{
public:
	struct Layer
	{
		std::shared_ptr<AudioClip> audioClip;
		XMFLOAT2 volumeRange;
		XMFLOAT2 pitchRange;
	};

private:
	XMFLOAT2 volumeRange = { 1.0f, 1.0f };
	XMFLOAT2 pitchRange = { 1.0f, 1.0f };
	std::vector<Sound::Layer> soundLayers = {};

public:
	Sound() = default;
	~Sound() = default;

	bool Create(
		XMFLOAT2 volumeRange,
		XMFLOAT2 pitchRange,
		const std::vector<Sound::Layer> &soundLayers
	);

	const Sound::Layer &GetLayer(size_t index) { return this->soundLayers[index]; }
	size_t GetLayerCount() const { return this->soundLayers.size(); }
};