#pragma once
#include "assets/types/texture_2d.hpp"
#include "assets/asset_loader.hpp"
#include "core/logging.hpp"

#include <unordered_map>
#include <string>
#include <memory>

class Renderer;

class AssetManager
{
private:
	AssetLoader assetLoader;
	std::unordered_map<std::string, std::weak_ptr<Asset>> assets;

public:
	bool Initialize(const Renderer *renderer);
	void Shutdown();

	template<class T>
	std::shared_ptr<T> Get(const std::string &path)
	{
		auto result = assets.find(path);

		if (result != assets.end())
		{
			if (auto asset = result->second.lock())
			{
				return std::static_pointer_cast<T>(asset);
			}
		}

		LogInfo("Loading %s", path.c_str());
		LogIndent();

		std::shared_ptr<T> asset = this->assetLoader.Load<T>(path);
		asset->SetPath(path);

		LogUnindent();

		assets[path] = asset;

		return asset;
	}

	std::shared_ptr<Texture2D> Load(const std::string &path);
};