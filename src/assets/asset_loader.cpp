#include "assets/asset_loader.hpp"
#include "assets/types/texture_2d.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"
#include "assets/types/model.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define TINYGLTF3_ENABLE_FS
#include "tinygltf/tiny_gltf_v3.h"

#include <sstream>

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

	auto texture = std::make_shared<Texture2D>(data, width, height, channels, path, this->renderer);

	return texture;
}

template<>
std::shared_ptr<Model> AssetLoader::Load<Model>(const std::string &path)
{
	tg3_parse_options opts = {};
	tg3_error_stack errors = {};
	tg3_model tg3Model = {};

	tg3_parse_options_init(&opts);
	tg3_error_stack_init(&errors);

	tg3_error_code error = tg3_parse_file(
		&tg3Model,
		&errors, 
		path.c_str(), 
		path.size(),
		&opts
	);
	
	if (error != TG3_OK)
	{
		std::stringstream errorMessage;
		for (uint32_t i = 0; i < errors.count; i++) 
		{
			errorMessage << (stderr, "[%d] %s\n", (int)errors.entries[i].severity,
				errors.entries[i].message ? errors.entries[i].message : "(null)");
		}
		LogError("Failed to parse %s: %s", path.c_str(), errorMessage.str().c_str());
	}

	tg3_model_free(&tg3Model);
	tg3_error_stack_free(&errors);

	std::shared_ptr<Model> model = std::make_shared<Model>();
	return model;
}

bool AssetLoader::Initialize(const Renderer *renderer)
{
	LogInfo("Creating Asset Loader...");
	LogIndent();

	if (!renderer)
	{
		LogError("Renderer is nullptr!");
		return false;
	}

    this->renderer = renderer;

	LogUnindent();

	return true;
}
