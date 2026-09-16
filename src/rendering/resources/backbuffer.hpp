#pragma once

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11UnorderedAccessView;
struct ID3D11Device;
struct IDXGISwapChain;

class BackBuffer
{
	ID3D11RenderTargetView *view{ nullptr };
	ID3D11UnorderedAccessView *uav{ nullptr };

public:
	BackBuffer() = default;
	BackBuffer(const BackBuffer &other) = delete;
	~BackBuffer();

	bool Create(ID3D11Device *device, IDXGISwapChain *swapChain);

	void Release();

	ID3D11RenderTargetView *GetView() { return this->view; }
	ID3D11UnorderedAccessView *GetUAV() { return this->uav; }
};