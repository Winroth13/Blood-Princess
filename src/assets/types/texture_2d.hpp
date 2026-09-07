#pragma once

#include "assets/asset.hpp"
#include "core/app_context.hpp"

#include <cstdint>
#include <d3d11.h>

class Renderer;

struct ID3D11Device;
struct ID3D11DeviceContext;

class Texture2D : public Asset
{
	ID3D11Texture2D *texture = nullptr;
	ID3D11ShaderResourceView *shaderResourceView = nullptr;
	
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;

	DXGI_FORMAT const GetDXGITextureFormat(uint32_t stbiChannels);

public:
	Texture2D(
		char* data, 
		const uint32_t width, 
		const uint32_t height, 
		const uint32_t channels,
		const std::string &filePath,
		const Renderer *renderer
	);
	~Texture2D();

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }
	uint32_t GetChannels() const { return this->channels; }
};