#pragma once
#include "assets/asset_loader.hpp"
#include "assets/texture.hpp"
#include "core/logging.hpp"

#include "assets/types/model.hpp"
#include "assets/types/texture_2d.hpp"
#include "assets/types/texture_cube.hpp"
#include "assets/types/audio_clip.hpp"

#include "assets/types/vertex_shader.hpp"
#include "assets/types/pixel_shader.hpp"
#include "assets/types/compute_shader.hpp"
#include "assets/types/geometry_shader.hpp"
#include "assets/types/domain_shader.hpp"
#include "assets/types/sound.hpp"

#include <unordered_map>
#include <string>
#include <memory>
#include <array>
#include <typeindex>

class Renderer;

class AssetManager
{
private:
	AssetLoader assetLoader;

	std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Asset>>> assets;

    std::array<std::shared_ptr<Texture2D>, +DefaultTexture::COUNT> defaultTextures;

    bool CreateDefaultTextures();

public:
	bool Initialize(const Renderer *renderer);
	void Shutdown();

	template<class T>
	std::shared_ptr<T> Get(const std::string &path)
	{
        std::unordered_map<std::string, std::shared_ptr<Asset>> &typedAssets = this->assets[std::type_index(typeid(T))];

		auto result = typedAssets.find(path);

		if (result != typedAssets.end())
			return static_pointer_cast<T>(result->second);

		LogInfo("Loading %s", path.c_str());
		LogIndent();

		std::shared_ptr<T> asset = this->assetLoader.Load<T>(path);

		LogUnindent();

        if (!asset) 
        {
            LogWarn("Failed to load '%s'", path.c_str());
            return nullptr;
        }

        asset->SetPath(path);
        typedAssets[path] = asset;

		return asset;
	}

    std::shared_ptr<Texture2D> GetDefaultTexture(DefaultTexture type) const { return this->defaultTextures[+type]; }
};