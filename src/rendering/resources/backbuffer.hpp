#pragma once

#include <cstdint>

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11DepthStencilView;
struct ID3D11Device;
struct IDXGISwapChain;

class BackBuffer
{
	uint32_t width = 0;
	uint32_t height = 0;

	ID3D11RenderTargetView *view = nullptr;
	ID3D11UnorderedAccessView *uav = nullptr;

	ID3D11Texture2D *depthStencilTexture = nullptr;
	ID3D11DepthStencilView *depthStencilView = nullptr;
	ID3D11ShaderResourceView *depthStencilSRV = nullptr;

public:
	BackBuffer() = default;
	BackBuffer(const BackBuffer &other) = delete;
	~BackBuffer();

	bool Create(
		ID3D11Device *device, 
		IDXGISwapChain *swapChain,
		uint32_t width,
		uint32_t height
	);

	void Release();

	ID3D11RenderTargetView *GetRenderTargetView() { return this->view; }
	ID3D11UnorderedAccessView *GetRenderTargetUAV() { return this->uav; }

	ID3D11DepthStencilView *GetDepthStencilView() { return this->depthStencilView; }
	ID3D11ShaderResourceView *GetDepthStencilSRV() { return this->depthStencilSRV; }

	uint32_t GetWidth() const { return this->width; }
	uint32_t GetHeight() const { return this->height; }
};