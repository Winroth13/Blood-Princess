#include "assets/asset_loader.hpp"
#include "assets/types/texture_2d.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

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

	auto texture = std::make_shared<Texture2D>(data, width, height, channels, path, this->appContext);

	return texture;
}

bool AssetLoader::Initialize(AppContext *appContext)
{
	LogInfo("Creating Asset Loader...");
	LogIndent();

	if (!appContext)
	{
		LogError("App Context is nullptr!");
		return false;
	}

	this->appContext = appContext;

	LogUnindent();

	return true;
}
