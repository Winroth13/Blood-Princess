#include "assets/asset_loader.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/model.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"
#include "assets/gltf_loader.hpp"
#include "rendering/renderer.hpp"
#include "assets/asset_manager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

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

	model->Create(loader.GetMeshes(), loader.GetMaterials());

	return model;
}

bool AssetLoader::Initialize(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	AssetManager *assetManager
)
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
