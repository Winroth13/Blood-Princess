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
#include "assets/types/font.hpp"
#include "assets/types/sound.hpp"
#include "assets/types/enemy.hpp"
#include "assets/types/floor.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define TINYGLTF3_ENABLE_FS
#include "tinygltf/tiny_gltf_v3.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis/stb_vorbis.c"

#include "yaml-cpp/yaml.h"

#include <sstream>
#include <fstream>
#include <d3d11.h>
#include <vector>

#include "yaml-cpp/yaml.h"
#include "serialization/custom_yaml_types.hpp"

#include <DirectXMath.h>
using namespace DirectX;

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

std::shared_ptr<Texture2D> AssetLoader::CreateTexture1x1(const uint8_t colour[4], bool isRGB, const std::string &name) 
{
    char *data = (char *)malloc(4);
    if (!data) 
    {
        LogWarn("Failed to allocate pixel data for '%s'", name.c_str());
        return nullptr;
    }

    data[0] = colour[0];
    data[1] = colour[1];
    data[2] = colour[2];
    data[3] = colour[3];

    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
    
    bool result = texture->Create(data, 1, 1, STBI_rgb_alpha, name, isRGB, this->device, this->deviceContext);
    if (!result) 
    {
        LogWarn("Failed to create 1x1 texture '%s'", name.c_str());
        return nullptr;
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
		loader.GetTextures(),
        loader.GetSkeleton(),
        loader.GetAnimations(),
		loader.GetBoundingBox()
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

template<>
std::shared_ptr<Font> AssetLoader::Load<Font>(const std::string &path) {
    std::ifstream reader(path, std::ios::binary | std::ios::ate);
    if (!reader.is_open()) {
        LogWarn("Could not open font file '%s'", path.c_str());
        return nullptr;
    }

    const std::streamsize size = reader.tellg();
    reader.seekg(0, std::ios::beg);

    std::vector<unsigned char> fontData(static_cast<size_t>(size));
    if (!reader.read(reinterpret_cast<char *>(fontData.data()), size)) {
        LogWarn("Could not read font file '%s'", path.c_str());
        return nullptr;
    }

    std::shared_ptr<Font> font = std::make_shared<Font>();
    if (!font->Create(device, std::move(fontData), AssetUtils::GetNameFromPath(path))) {
        LogWarn("Failed to create font from file '%s'", path.c_str());
        return nullptr;
    }

    return font;
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

	RenderUtils::SetDebugName(vertexShader->GetShader(), DebugCategory::SHADER, "VS", shaderName);

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

	RenderUtils::SetDebugName(pixelShader->GetShader(), DebugCategory::SHADER, "PS", shaderName);

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

	RenderUtils::SetDebugName(computeShader->GetShader(), DebugCategory::SHADER, "CS", shaderName);

	return computeShader;
}

template<>
std::shared_ptr<GeometryShader> AssetLoader::Load<GeometryShader>(const std::string &path)
{
	std::string byteCode;

	if (!LoadShaderByteCode(path, byteCode))
	{
		LogError("Failed to load byte shader code from %s", path.c_str());
	}

	std::shared_ptr<GeometryShader> shader = std::make_shared<GeometryShader>();
	if (!shader->Create(byteCode, this->device))
	{
		LogError("Failed to create Geometry Shader");
	}

	std::string shaderName = AssetUtils::GetNameFromPath(path);
	shaderName = AssetUtils::FormatShaderName(shaderName);

	RenderUtils::SetDebugName(shader->GetShader(), DebugCategory::SHADER, "GS", shaderName);

	return shader;
}

template<>
std::shared_ptr<DomainShader> AssetLoader::Load<DomainShader>(const std::string &path)
{
	std::string byteCode;

	if (!LoadShaderByteCode(path, byteCode))
	{
		LogError("Failed to load byte shader code from %s", path.c_str());
	}

	std::shared_ptr<DomainShader> domainShader = std::make_shared<DomainShader>();
	if (!domainShader->Create(byteCode, this->device))
	{
		LogError("Failed to create Domain Shader");
	}

	std::string shaderName = AssetUtils::GetNameFromPath(path);
	shaderName = AssetUtils::FormatShaderName(shaderName);

	RenderUtils::SetDebugName(domainShader->GetShader(), DebugCategory::SHADER, "DS", shaderName);

	return domainShader;
}

#pragma endregion

template<>
std::shared_ptr<Sound> AssetLoader::Load<Sound>(const std::string &path)
{
	std::shared_ptr<Sound> sound = std::make_shared<Sound>();

	try
	{
		YAML::Node file = YAML::LoadFile(path);

		if (!file.IsDefined() || file.IsNull())
		{
			LogWarn("File is empty: %s", path.c_str());
			return sound;
		}

		std::vector<Sound::Layer> layers = {};
		XMFLOAT2 volumeRange = { 1.0f, 1.0f };
		XMFLOAT2 pitchRange = { 1.0f, 1.0f };

		if (YAML::Node node = file["sound"])
		{
			if (node.IsSequence())
			{
				for (YAML::Node layer : node)
				{
					std::string fileName = layer["file"].as<std::string>("");
					float minLayerVolume = layer["min_volume"].as<float>(1.0f);
					float maxLayerVolume = layer["max_volume"].as<float>(1.0f);
					float minLayerPitch = layer["min_pitch"].as<float>(1.0f);
					float maxLayerPitch = layer["max_pitch"].as<float>(1.0f);

					Sound::Layer soundLayer = {};
					soundLayer.audioClip = AssetLoader::Load<AudioClip>(fileName);
					soundLayer.volumeRange = { minLayerVolume, maxLayerVolume };
					soundLayer.pitchRange = { minLayerPitch, maxLayerPitch };

					layers.push_back(soundLayer);
				}
			}
		}

		sound->Create(volumeRange, pitchRange, layers);
		return sound;
	}
	catch (const YAML::BadFile e)
	{
		LogWarn("Failed to open file: %s", path.c_str());
		return sound;
	}
	catch (const YAML::ParserException e)
	{
		LogWarn("Failed to parse YAML syntax: %s", e.what());
		return sound;
	}
	catch (const std::exception e)
	{
		LogError("An error occurred: %s", e.what());
		return sound;
	}

	return sound;
}

template<>
std::shared_ptr<Enemy> AssetLoader::Load<Enemy>(const std::string &path)
{
	std::shared_ptr<Enemy> enemy = std::make_shared<Enemy>();

	try
	{
		YAML::Node file = YAML::LoadFile(path);

		if (!file.IsDefined() || file.IsNull())
		{
			LogWarn("File is empty: %s", path.c_str());
			return enemy;
		}

		enemy->type = Enemy::GetEnemyTypeFromString(file["type"].as<std::string>(""));
		enemy->health = file["health"].as<float>(100.0f);
		enemy->score = file["score"].as<int>(1);
		enemy->scale = file["scale"].as<float>(1.0f);

		enemy->avoidanceRadius = file["avoidance_radius"].as<float>(1.0f);
		enemy->moveSpeed = file["move_speed"].as<float>(1.0f);

		enemy->knockbackFactor = file["knockback_factor"].as<float>(1.0f);
		enemy->knockbackDecay = file["knockback_decay"].as<float>(1.0f);
		enemy->knockbackStunDuration = file["knockback_stun_duration"].as<float>(0.0f);

		enemy->minRange = file["min_range"].as<float>(0.0f);
		enemy->maxRange = file["max_range"].as<float>(0.5f);

		enemy->attackTime = file["attack_time"].as<float>(1.0f);

		enemy->model = this->assetManager->Get<Model>(file["model"].as<std::string>(""));
		enemy->walkAnimation = file["walk_animation"].as<std::string>("");
		enemy->attackAnimation = file["attack_animation"].as<std::string>("");

		enemy->extends = file["extends"].as<XMFLOAT3>(XMFLOAT3(1.0f, 1.0f, 1.0f));
		enemy->extendsOffset = file["extends_offset"].as<XMFLOAT3>(XMFLOAT3(1.0f, 1.0f, 1.0f));

		enemy->startAttackSound = this->assetManager->Get<Sound>(file["start_attack_sound"].as<std::string>(""));
		enemy->attackSound = this->assetManager->Get<Sound>(file["attack_sound"].as<std::string>(""));
		enemy->hitSound = this->assetManager->Get<Sound>(file["hit_sound"].as<std::string>(""));
		enemy->deathSound = this->assetManager->Get<Sound>(file["death_sound"].as<std::string>(""));
		enemy->barkSound = this->assetManager->Get<Sound>(file["bark_sound"].as<std::string>(""));

		std::vector<EnemyAttack> attacks = {};

		for (YAML::Node attackNode : file["attacks"])
		{
			EnemyAttack attack = {};
			attack.name = attackNode["name"].as<std::string>("");
			
			std::string attackTypeStr = attackNode["type"].as<std::string>("");
			if (attackTypeStr == "Melee")
			{
				attack.type = AttackType::Melee;
			}
			else if (attackTypeStr == "Projectile")
			{
				attack.type = AttackType::Projectile;
			}
			else
			{
				LogWarn("Invalid attack type: %s", attackTypeStr.c_str());
			}

			attack.animation = attackNode["animation"].as<std::string>("");
			attack.sound = this->assetManager->Get<Sound>(file["sound"].as<std::string>(""));
			attack.damage = attackNode["damage"].as<float>(1.0f);
			attack.knockback = attackNode["knockback"].as<float>(1.0f);
			attack.hitDelay = attackNode["hit_delay"].as<float>(1.0f);
			attack.hitDuration = attackNode["hit_duration"].as<float>(1.0f);
			attack.range = attackNode["range"].as<float>(1.0f);

			attack.extends = attackNode["extends"].as<XMFLOAT3>(XMFLOAT3(0.5f, 0.5f, 0.5f));
			attack.extendsOffset = attackNode["extends_offset"].as<XMFLOAT3>(XMFLOAT3(0.0f, 0.0f, 0.0f));
			attack.socket = attackNode["socket"].as<std::string>("");

			attacks.push_back(attack);
		}

		enemy->attacks = attacks;
	}
	catch (const YAML::BadFile e)
	{
		LogError("Failed to open file: %s", path.c_str());
		return enemy;
	}
	catch (const YAML::ParserException e)
	{
		LogError("Failed to parse YAML syntax: %s", e.what());
		return enemy;
	}
	catch (const std::exception e)
	{
		LogError("An error occurred: %s", e.what());
		return enemy;
	}

	return enemy;
}

template<>
std::shared_ptr<Floor> AssetLoader::Load<Floor>(const std::string &path)
{
	std::shared_ptr<Floor> floor = std::make_shared<Floor>();

	try
	{
		YAML::Node file = YAML::LoadFile(path);

		XMFLOAT4 hsv = { 0.0f, 0.0f, 0.0f, 0.0f };
		hsv.x = file["hue"].as<float>(0.0f);
		hsv.y = file["saturation"].as<float>(1.0f);
		hsv.z = file["value"].as<float>(1.0f);
		hsv.w = file["intensity"].as<float>(0.0f);

		floor->SetHSV(hsv);

		std::vector<Wave> waves = {};

		if (YAML::Node wavesNode = file["waves"])
		{
			for (YAML::Node waveNode : wavesNode)
			{
				Wave wave = {};
				wave.completion = waveNode["completion"].as<float>(0.75f);

				if (YAML::Node enemiesNode = waveNode["enemies"])
				{
					for (auto enemyNode : enemiesNode)
					{
						WaveEnemy enemy = {};
						enemy.type = Enemy::GetEnemyTypeFromString(enemyNode["type"].as<std::string>(""));
						enemy.amount = static_cast<uint32_t>(enemyNode["amount"].as<int>(0));
						enemy.time = enemyNode["time"].as<float>(30.0f);

						wave.enemies.push_back(enemy);
					}
				}

				waves.push_back(wave);
			}
		}

		floor->SetWaves(waves);
	}
	catch (const YAML::BadFile e)
	{
		LogError("Failed to open file: %s", path.c_str());
		return floor;
	}
	catch (const YAML::ParserException e)
	{
		LogError("Failed to parse YAML syntax: %s", e.what());
		return floor;
	}
	catch (const std::exception e)
	{
		LogError("An error occurred: %s", e.what());
		return floor;
	}

	return floor;
}