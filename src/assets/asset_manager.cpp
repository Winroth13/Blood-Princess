#include "asset_manager.hpp"
#include "core/logging.hpp"
#include "rendering/renderer.hpp"

bool AssetManager::CreateDefaultTextures()
{
    LogInfo("Creating Default Textures...");
    LogIndent();

    struct DefaultTextureDesc
    {
        DefaultTexture type;
        uint8_t colour[4];
        bool isRGB;
        std::string name;
    };

    const DefaultTextureDesc descs[+DefaultTexture::COUNT] =
    {
        { DefaultTexture::ALBEDO,             { 255, 255, 255, 255 }, true,  "Default Albedo"             },
        { DefaultTexture::NORMAL,             { 128, 128, 255, 255 }, false, "Default Normal"             },
        { DefaultTexture::METALLIC_ROUGHNESS, { 0  , 255, 0  , 255 }, false, "Default Metallic Roughness" }
    };

    for (const DefaultTextureDesc &desc : descs)
    {
        std::shared_ptr<Texture2D> texture = this->assetLoader.CreateTexture1x1(desc.colour, desc.isRGB, desc.name);
        if (!texture)
        {
            LogError("Failed to create default texture '%s'", desc.name.c_str());
            return false;
        }

        texture->SetPath(desc.name);
        this->defaultTextures[+desc.type] = texture;
    }

    LogUnindent();
    return true;
}

bool AssetManager::Initialize(const Renderer *renderer)
{
	LogInfo("Creating Asset Manager...");
	LogIndent();

    if (!this->assetLoader.Initialize(renderer->GetDevice(), renderer->GetDeviceContext(), this))
    {
		LogError("Asset Loader failed to initialize");
        return false;
    }

    if (!this->CreateDefaultTextures()) 
        return false;

	LogUnindent();

	return true;
}

void AssetManager::Shutdown()
{
	LogInfo("Shutting down Asset Manager...");

    // Log Leaked Assets
    for (auto &typeIndexPair : this->assets)
    {
        for (auto &assetPair : typeIndexPair.second)
        {
            if (assetPair.second.use_count() > 1)
            {
                LogWarn("%s leaked", assetPair.first.c_str());
            }
        }
    }

    for (std::shared_ptr<Texture2D> &texture : this->defaultTextures)
        texture.reset();
}
