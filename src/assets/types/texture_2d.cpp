#include "assets/types/texture_2d.hpp"
#include "core/logging.hpp"
#include "stb_image/stb_image.h"

#include "rendering/render_utils.hpp"
#include "assets/asset_utils.hpp"

#include <d3d11.h>

// Convert STBI channels to DXGI Texture Format.
DXGI_FORMAT const Texture2D::GetDXGITextureFormat(uint32_t stbiChannels)
{
	switch (stbiChannels)
	{
	case STBI_rgb:
	case STBI_rgb_alpha:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case STBI_grey:
		return DXGI_FORMAT_R8_UNORM;
		
	default:
		LogWarn("Texture has unsupported format: %d (falling back to RGBA)", stbiChannels);
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

bool Texture2D::Create(
	char *data,
	const uint32_t width,
	const uint32_t height,
	const uint32_t channels,
	const std::string &name,
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext
)
{
	this->width = width;
	this->height = height;
	this->channels = channels;

	DXGI_FORMAT format = GetDXGITextureFormat(this->channels);

	/* Create D3D11 Texture */
	D3D11_TEXTURE2D_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.Width = this->width;
	desc.Height = this->height;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// To generate mips, the texture must be created empty
	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);

	if (FAILED(hr))
	{
		LogError("DirectX failed to create Texture2D for texture: %s", name.c_str());
        return false;
	}

	RenderUtils::SetDebugName(this->texture, "Texture", name);

	LogInfo("DirectX Texture created");

	UINT rowPitch = this->width * sizeof(char) * 4;
	UINT imageSize = rowPitch * this->height;

	deviceContext->UpdateSubresource(this->texture, 0, nullptr, data, rowPitch, imageSize);

	stbi_image_free(data);

	// Generate mips.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;

	hr = device->CreateShaderResourceView(this->texture, &srvDesc, &this->shaderResourceView);

	if (FAILED(hr))
	{
		LogError("DirectX failed to shader resource view for texture: %s", name.c_str());
        return false;
	}

	LogInfo("Shader Resource View created");

	deviceContext->GenerateMips(this->shaderResourceView);

	RenderUtils::SetDebugName(this->shaderResourceView, "Texture", name, "SRV");

	return true;
}

Texture2D::~Texture2D()
{
	LogInfo("Unloading %s", this->path.c_str());
	LogIndent();

	LogInfo("Releasing Shader Resource View");
	SafeRelease(shaderResourceView);
	LogInfo("Releasing DirectX Texture");

	LogUnindent();

	SafeRelease(texture);
}