#include "asset_manager.hpp"
#include "core/logging.hpp"

#include "assets/loaders/texture_2d_loader.hpp"

bool AssetManager::Initialize(AppContext *appContext)
{
	LogInfo("Creating Asset Manager...");

	this->appContext = appContext;
	if (!appContext)
	{
		LogError("App Context was nullptr");
		return false;
	}

	return true;
}

void AssetManager::Shutdown()
{
	LogInfo("Shutting down Asset Manager...");
}

Texture2D *AssetManager::GetTexture2D(const std::string &filePath)
{
	if (this->texture2dAssets.find(filePath) == this->texture2dAssets.end())
	{
		texture2dAssets[filePath] = std::unique_ptr<Texture2D>(
			Texture2DLoader::Load(filePath, appContext)
		);
	}
	return texture2dAssets[filePath].get();
}