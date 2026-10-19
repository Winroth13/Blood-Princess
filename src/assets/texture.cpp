#include "assets/texture.hpp"
#include "core/logging.hpp"
#include "stb_image/stb_image.h"

#include "rendering/render_utils.hpp"
#include "assets/asset_utils.hpp"

#include <d3d11.h>

// Convert STBI channels to DXGI Texture Format.
DXGI_FORMAT const Texture::GetDXGITextureFormat(uint32_t stbiChannels, bool isColor)
{
	switch (stbiChannels)
	{
	case STBI_rgb:
	case STBI_rgb_alpha:
		return isColor ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

	case STBI_grey:
		return DXGI_FORMAT_R8_UNORM;

	default:
		LogWarn("Texture has unsupported format: %d (falling back to RGBA)", stbiChannels);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

bool Texture::Create(
	char **data,
	const uint32_t width,
	const uint32_t height,
	const uint32_t channels,
	const std::string &name,
	const std::string &type,
	bool isTextureCube,
	bool isRGB,
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext
)
{
	this->width = width;
	this->height = height;
	this->channels = channels;

	int numTextures = isTextureCube ? 6 : 1;

	DXGI_FORMAT format = GetDXGITextureFormat(this->channels, isRGB);

	/* Create D3D11 Texture */
	D3D11_TEXTURE2D_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MipLevels = isTextureCube ? 1 : 0;
	desc.ArraySize = numTextures;
	desc.Format = format;
	desc.Width = this->width;
	desc.Height = this->height;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = isTextureCube ? D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE : D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// To generate mips, the texture must be created empty
	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create %s for texture: %s", type.c_str(), name.c_str());
		return false;
	}

	RenderUtils::SetDebugName(this->texture, DebugCategory::TEXTURE, type, name);

	LogInfo("DirectX %s created", type.c_str());

	UINT rowPitch;
	UINT imageSize;
	for (int i = 0; i < numTextures; i++)
	{
		rowPitch = this->width * sizeof(char) * 4;
		imageSize = rowPitch * this->height;

		deviceContext->UpdateSubresource(this->texture, i, nullptr, data[i], rowPitch, imageSize);

		stbi_image_free(data[i]);
	}

	// Generate mips.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	
	if (isTextureCube)
	{
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
	}
	else
	{
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
	}

	hr = device->CreateShaderResourceView(this->texture, &srvDesc, &this->shaderResourceView);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create shader resource view for texture: %s", name.c_str());
		return false;
	}

	LogInfo("Shader Resource View created");

	if (!isTextureCube)
		deviceContext->GenerateMips(this->shaderResourceView);

	RenderUtils::SetDebugName(this->shaderResourceView, DebugCategory::TEXTURE, type, name, "SRV");

	return true;
}

Texture::~Texture()
{
	LogInfo("Unloading %s", this->path.c_str());
	LogIndent();

	LogInfo("Releasing Shader Resource View");
	SafeRelease(shaderResourceView);
	LogInfo("Releasing DirectX Texture");

	LogUnindent();

	SafeRelease(texture);
}