#include "backbuffer.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

#include <d3d11.h>

bool BackBuffer::Create(
	ID3D11Device* device, 
	IDXGISwapChain *swapChain,
	uint32_t width,
	uint32_t height
)
{
	LogInfo("Creating Back Buffer...");
	LogIndent();

#pragma region Render Target
	ID3D11Texture2D *texture{};
	HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&texture);

	if (!texture)
	{
		LogError("Texture is nullptr!");
		return false;
	}

	RenderUtils::SetDebugName(texture, DebugCategory::CORE,"Backbuffer");

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

	RenderUtils::SetDebugName(this->view, DebugCategory::CORE, "", "Backbuffer", "RTV");

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

	RenderUtils::SetDebugName(this->uav, DebugCategory::CORE, "", "Backbuffer", "UAV");
#pragma endregion

#pragma region Depth Stencil
	{

		HRESULT result{};

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		result = device->CreateTexture2D(&textureDesc, nullptr, &this->depthStencilTexture);

		if (FAILED(result))
		{
			LogError("Failed to create the Depth Stencil Texture");
			return false;
		}

		RenderUtils::SetDebugName(this->depthStencilTexture, DebugCategory::RENDER_PASS, "Depth Stencil", "Deferred");

		LogInfo("Depth Stencil Texture created");

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		result = device->CreateDepthStencilView(this->depthStencilTexture, &depthStencilViewDesc, &this->depthStencilView);

		if (FAILED(result))
		{
			LogError("Failed to create the Depth Stencil View");
			return false;
		}

		RenderUtils::SetDebugName(this->depthStencilView, DebugCategory::RENDER_PASS, "Depth Stencil", "Deferred", "DSV");

		LogInfo("Depth Stencil View created");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		result = device->CreateShaderResourceView(this->depthStencilTexture, &srvDesc, &this->depthStencilSRV);

		if (FAILED(result))
		{
			LogError("Failed to create the Depth Stencil Shader Resource View");
			return false;
		}

		RenderUtils::SetDebugName(this->depthStencilView, DebugCategory::RENDER_PASS, "Depth Stencil", "Deferred", "SRV");

		LogInfo("Depth Stencil Shader Resource View created");
	}
#pragma endregion

	LogUnindent();

	return true;
}

void BackBuffer::Release()
{
	SafeRelease(this->uav);
	SafeRelease(this->view);

	SafeRelease(this->depthStencilSRV);
	SafeRelease(this->depthStencilView);
	SafeRelease(this->depthStencilTexture);
}

BackBuffer::~BackBuffer()
{
	this->Release();
}