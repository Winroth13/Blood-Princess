#include "texture_2d_loader.hpp"
#include "core/logging.hpp"
#include "core/app_context.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include <d3d11.h>

Texture2D *Texture2DLoader::Load(const std::string &filePath, AppContext* appContext)
{
	LogInfo("Loading %s", filePath.c_str());
	LogIndent();

	int width, height, channels;
	// TODO: Maybe we shouldn't hardcode desired channels to be four (RGBA).
	char *data = (char*)stbi_load(filePath.c_str(), &width, &height, &channels, 4);

	if (data == nullptr)
	{
		LogWarn("Failed to load image data from file: %s (using fallback)", filePath.c_str());
		// Use fallback texture instead.
		// TODO: Maybe not load from file.
		data = (char*)stbi_load("fallback.png", &width, &height, &channels, 4);

		// TODO: Remove once fallback has a place to stay or is built.
		if (data == nullptr)
		{
			LogError("Failed to load fallback texture");
		}
	}

	Texture2D *texture = new Texture2D(data, width, height, channels, filePath, appContext);

	LogUnindent();

	return texture;
}