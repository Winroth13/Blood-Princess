#pragma once
#include <cstdint>
#include <d3d11.h>
#include <string>

class GBuffer
{
	ID3D11Texture2D *texture = nullptr;
	ID3D11RenderTargetView *rtv = nullptr;
	ID3D11ShaderResourceView *srv = nullptr;
    ID3D11UnorderedAccessView *uav = nullptr;
	D3D11_VIEWPORT viewport{};

public:
	GBuffer() = default;
	~GBuffer();

	bool Initialize(
		ID3D11Device* device,
		uint32_t width, 
		uint32_t height,
        DXGI_FORMAT format,
		std::string name
	);

	ID3D11Texture2D *GetTexture() const { return this->texture; }
	ID3D11RenderTargetView *GetRenderTargetView() const { return this->rtv; }
	ID3D11ShaderResourceView *GetShaderResourceView() const { return this->srv; }
    ID3D11UnorderedAccessView *GetUnorderedAccessView() const { return this->uav; }

	const D3D11_VIEWPORT &GetViewport() const { return viewport; }
};