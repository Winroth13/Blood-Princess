#include "assets/asset_loader.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/model.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"
#include "assets/types/model.hpp"
#include "assets/types/audio_clip.hpp"
#include <stdexcept>
#include "assets/gltf_loader.hpp"
#include "rendering/renderer.hpp"
#include "assets/asset_manager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define TINYGLTF3_ENABLE_FS
#include "tinygltf/tiny_gltf_v3.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis/stb_vorbis.c"

#include <sstream>

#include <d3d11.h>

template<>
std::shared_ptr<Texture2D> AssetLoader::Load<Texture2D>(const std::string &path)
{
	int width, height, channels;
	// TODO: Maybe we shouldn't hardcode desired channels to be four (RGBA).
	char *data = (char *)stbi_load(path.c_str(), &width, &height, &channels, 4);

	if (data == nullptr)
	{
		LogWarn("Failed to load image data from file: %s (using fallback)", path.c_str());
		// Use fallback texture instead.
		// TODO: Maybe not load from file.
		data = (char *)stbi_load("fallback.png", &width, &height, &channels, 4);

		// TODO: Remove once fallback has a place to stay or is built.
		if (data == nullptr)
		{
			LogError("Failed to load fallback texture");
		}
	}

	auto texture = std::make_shared<Texture2D>(
		data, width, height, 
		channels, path, 
		this->device, this->deviceContext
	);

	return texture;
}

template<>
std::shared_ptr<Model> AssetLoader::Load<Model>(const std::string &path)
{
	std::shared_ptr<Model> model = std::make_shared<Model>();
	GLTFLoader loader;

	if (!loader.Load(path, this->device, this->deviceContext, this->assetManager))
	{
		LogError("Failed to parse file");
		return model;
	}

	model->Create(
		loader.GetMeshes(), 
		loader.GetMaterials(), 
		loader.GetTextures()
	);

	return model;
}

template<>
std::shared_ptr<AudioClip> AssetLoader::Load<AudioClip>(const std::string& path)
{
	int channels = 0;
	int sampleRate = 0;
	short *output = nullptr;


	int samplesPerChannel = stb_vorbis_decode_filename(
		path.c_str(),
		&channels,
		&sampleRate,
		&output
	);

	if (samplesPerChannel <= 0 || output == nullptr)
	{
		LogError("Failed to load or decode audio file: %s", path.c_str());
		return nullptr;
	}

	uint32_t totalSamples = static_cast<uint32_t>(samplesPerChannel * channels);

	std::shared_ptr<AudioClip> audioClip = std::make_shared<AudioClip>();

	if (!audioClip->Create(output, totalSamples, static_cast<uint32_t>(channels), static_cast<uint32_t>(sampleRate)))
	{
		free(output);
		LogError("Failed to initialize AudioClip for: %s", path.c_str());
		return nullptr;
	}

	free(output);

	LogInfo("Loaded audio %s: %d Hz, %d channel(s), %d total samples",
		path.c_str(), sampleRate, channels, totalSamples);

	return audioClip;
}

bool AssetLoader::Initialize(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	AssetManager *assetManager)
{
	LogInfo("Creating Asset Loader...");
	LogIndent();

	if (!device)
	{
		LogError("Device is nullptr!");
		return false;
	}
	this->device = device;

	if (!deviceContext)
	{
		LogError("Device Context is nullptr!");
		return false;
	}
	this->deviceContext = deviceContext;

	if (!assetManager)
	{
		LogError("Asset Manager is nullptr!");
		return false;
	}
	this->assetManager = assetManager;

	LogUnindent();

	return true;
}
