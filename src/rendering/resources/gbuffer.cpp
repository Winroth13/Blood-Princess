#include "gbuffer.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

bool GBuffer::Initialize(
	ID3D11Device *device,
	uint32_t width,
	uint32_t height,
	DXGI_FORMAT format
)
{
	LogInfo("Creating GBuffer...");
	LogIndent();

	if (!device)
	{
		LogError("Device is nullptr");
		return false;
	}

	HRESULT result{};

#pragma region Viewport
	this->viewport.TopLeftX = 0;
	this->viewport.TopLeftY = 0;
	this->viewport.Width = static_cast<float>(width);
	this->viewport.Height = static_cast<float>(height);
	this->viewport.MinDepth = 0;
	this->viewport.MaxDepth = 1;
#pragma endregion

#pragma region Texture
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		result = device->CreateTexture2D(&desc, nullptr, &this->texture);

		if (FAILED(result))
		{
			LogError("Failed to create DirectX11 Texture2D");
			return false;
		}

		result = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);

		if (FAILED(result))
		{
			LogError("Failed to create Shader Resource View");
			return false;
		}

		result = device->CreateRenderTargetView(this->texture, nullptr, &this->rtv);

		if (FAILED(result))
		{
			LogError("Failed to create Render Target View");
			return false;
		}
	}
#pragma endregion

	LogUnindent();

	return true;
}

GBuffer::~GBuffer()
{
	SafeRelease(this->texture);
	SafeRelease(this->srv);
	SafeRelease(this->rtv);
}