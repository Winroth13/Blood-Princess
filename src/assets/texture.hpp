#pragma once

#include "assets/asset.hpp"
#include "core/app_context.hpp"

#include <cstdint>
#include <d3d11.h>

class Renderer;

struct ID3D11Device;
struct ID3D11DeviceContext;

enum class DefaultTexture
{
	ALBEDO,
	NORMAL,
	METALLIC_ROUGHNESS,

	COUNT
};

constexpr int operator+(DefaultTexture type) noexcept
{
	return static_cast<int>(type);
}

class Texture : public Asset
{
	ID3D11Texture2D *texture = nullptr;
	ID3D11ShaderResourceView *shaderResourceView = nullptr;

	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t channels = 0;

	DXGI_FORMAT const GetDXGITextureFormat(uint32_t stbiChannels, bool isRGB);

protected:
	bool Create(
		char **data,
		const uint32_t width,
		const uint32_t height,
		const uint32_t channels,
		const std::string &name,
		const std::string &type,
		int numTexures,
		bool isRGB,
		ID3D11Device *device,
		ID3D11DeviceContext *deviceContext
	);

public:
	Texture() = default;
	~Texture();

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }
	uint32_t GetChannels() const { return this->channels; }
	ID3D11ShaderResourceView *GetShaderResourceView() const { return this->shaderResourceView; }
};