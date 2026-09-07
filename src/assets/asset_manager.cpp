#include "asset_manager.hpp"
#include "core/logging.hpp"

bool AssetManager::Initialize(const Renderer *renderer)
{
	LogInfo("Creating Asset Manager...");
	LogIndent();

	if (!assetLoader.Initialize(renderer))
		LogError("Asset Loader failed to initialize");

	LogUnindent();

	return true;
}

void AssetManager::Shutdown()
{
	LogInfo("Shutting down Asset Manager...");
}
