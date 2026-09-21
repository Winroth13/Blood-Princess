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
#include "rendering/render_utils.hpp"
#include "assets/asset_utils.hpp"

#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"
#include "assets/types/compute_shader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define TINYGLTF3_ENABLE_FS
#include "tinygltf/tiny_gltf_v3.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis/stb_vorbis.c"

#include <sstream>
#include <fstream>
#include <d3d11.h>

template<>
std::shared_ptr<Texture2D> AssetLoader::Load<Texture2D>(const std::string &path)
{
	int width, height, channels;
	// TODO: Maybe we shouldn't hardcode desired channels to be four (RGBA)
	char *data = (char *)stbi_load(path.c_str(), &width, &height, &channels, 4);

	if (data == nullptr)
	{
		LogWarn("Failed to load image data from file: %s (using fallback)", path.c_str());
		// Use fallback texture instead
		// TODO: Maybe not load from file
		data = (char *)stbi_load("assets/images/fallback.png", &width, &height, &channels, 4);

		// TODO: Remove once fallback has a place to stay or is built
		if (data == nullptr)
		{
			LogError("Failed to load fallback texture");
		}
	}

	auto texture = std::make_shared<Texture2D>();

	bool result = texture->Create(
		data, width, height,
		channels, path, false,
		this->device, this->deviceContext
	);

	if (!result)
	{
		LogError("Failed to create Texture2D for: %s", path.c_str());
	}

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

	bool result = model->Create(
		loader.GetMeshes(), 
		loader.GetMaterials(), 
		loader.GetTextures()
	);

	if (!result)
	{
		LogError("Failed to create Model for: %s", path.c_str());
	}

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

#pragma region Shaders

static bool LoadShaderByteCode(const std::string &path, std::string& byteCode)
{
	std::ifstream reader{};
	reader.open(path, std::ios::binary | std::ios::ate);

	if (!reader.is_open())
	{
		LogError("Could not open shader file: %s", path.c_str());
		return false;
	}

	reader.seekg(0, std::ios::end);
	byteCode.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	byteCode.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);
	return true;
}

template<>
std::shared_ptr<VertexShader> AssetLoader::Load<VertexShader>(const std::string &path)
{
	std::string byteCode;

	if (!LoadShaderByteCode(path, byteCode))
	{
		LogError("Failed to load byte shader code from %s", path.c_str());
	}

	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>();
	if (!vertexShader->Create(byteCode, this->device))
	{
		LogError("Failed to create Vertex Shader");
	}

	std::string shaderName = AssetUtils::GetNameFromPath(path);
	shaderName = AssetUtils::FormatShaderName(shaderName);

	RenderUtils::SetDebugName(vertexShader->GetShader(), "VS", shaderName);

	return vertexShader;
}

template<>
std::shared_ptr<PixelShader> AssetLoader::Load<PixelShader>(const std::string &path)
{
	std::string byteCode;

	if (!LoadShaderByteCode(path, byteCode))
	{
		LogError("Failed to load byte shader code from %s", path.c_str());
	}

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>();
	if (!pixelShader->Create(byteCode, this->device))
	{
		LogError("Failed to create Pixel Shader");
	}

	std::string shaderName = AssetUtils::GetNameFromPath(path);
	shaderName = AssetUtils::FormatShaderName(shaderName);

	RenderUtils::SetDebugName(pixelShader->GetShader(), "PS", shaderName);

	return pixelShader;
}

template<>
std::shared_ptr<ComputeShader> AssetLoader::Load<ComputeShader>(const std::string &path)
{
	std::string byteCode;

	if (!LoadShaderByteCode(path, byteCode))
	{
		LogError("Failed to load byte shader code from %s", path.c_str());
	}

	std::shared_ptr<ComputeShader> computeShader = std::make_shared<ComputeShader>();
	if (!computeShader->Create(byteCode, this->device))
	{
		LogError("Failed to create Compute Shader");
	}

	std::string shaderName = AssetUtils::GetNameFromPath(path);
	shaderName = AssetUtils::FormatShaderName(shaderName);

	RenderUtils::SetDebugName(computeShader->GetShader(), "CS", shaderName);

	return computeShader;
}

#pragma endregion