#pragma once
#include <cstdint>
#include <d3d11.h>

class GBuffer
{
	ID3D11Texture2D *texture{ nullptr };
	ID3D11RenderTargetView *rtv{ nullptr };
	ID3D11ShaderResourceView *srv{ nullptr };
	D3D11_VIEWPORT viewport{};

public:
	GBuffer() = default;
	~GBuffer();

	bool Initialize(
		ID3D11Device* device,
		uint32_t width, 
		uint32_t height
	);

	ID3D11Texture2D *GetTexture() { return this->texture; }
	ID3D11RenderTargetView *GetRenderTargetView() { return this->rtv; }
	ID3D11ShaderResourceView *GetShaderResourceView() { return this->srv; }

	const D3D11_VIEWPORT &GetViewport() const { return viewport; }
};