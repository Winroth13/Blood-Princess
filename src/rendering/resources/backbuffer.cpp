#include "backbuffer.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include <d3d11.h>

bool BackBuffer::Create(ID3D11Device* device, IDXGISwapChain *swapChain)
{
	LogInfo("Creating Back Buffer...");
	LogIndent();

	ID3D11Texture2D *texture{};
	HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&texture);

	if (!texture)
	{
		LogError("Texture is nullptr!");
		return false;
	}

	HRESULT hr = device->CreateRenderTargetView(
		texture,
		nullptr,
		&this->view
	);

	if (FAILED(hr))
	{
		LogError("Failed to create Render Target View for the Backbuffer");
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.Texture2DArray.MipSlice = 0;
	uavDesc.Texture2DArray.ArraySize = 1;

	hr = device->CreateUnorderedAccessView(texture, &uavDesc, &this->uav);
	texture->Release();
	if (FAILED(hr))
	{
		LogError("Failed to create Unordered Access View for the Backbuffer");
		return false;
	}

	LogUnindent();

	return true;
}

void BackBuffer::Release()
{
	SafeRelease(this->uav);
	SafeRelease(this->view);
}

BackBuffer::~BackBuffer()
{
	this->Release();
}