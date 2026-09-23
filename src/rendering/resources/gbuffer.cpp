#include "gbuffer.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

bool GBuffer::Initialize(
	ID3D11Device *device,
	uint32_t width,
	uint32_t height,
	DXGI_FORMAT format,
	std::string name
)
{
	LogInfo("Creating GBuffer (%u, %u)...", width, height);
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
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		result = device->CreateTexture2D(&desc, nullptr, &this->texture);
		if (FAILED(result))
		{
			LogError("Failed to create DirectX11 Texture2D");
			return false;
		}
		RenderUtils::SetDebugName(this->texture, DebugCategory::GBUFFER, "GBuffer", name);

		result = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
		if (FAILED(result))
		{
			LogError("Failed to create Shader Resource View");
			return false;
		}
		RenderUtils::SetDebugName(this->srv, DebugCategory::GBUFFER, "GBuffer", name, "SRV");

		result = device->CreateRenderTargetView(this->texture, nullptr, &this->rtv);
		if (FAILED(result))
		{
			LogError("Failed to create Render Target View");
			return false;
		}
	
		RenderUtils::SetDebugName(this->rtv, DebugCategory::GBUFFER, "GBuffer", name, "RTV");

        result = device->CreateUnorderedAccessView(this->texture, nullptr, &this->uav);
        if (FAILED(result))
        {
            LogError("Failed to create Unordered Access View");
            return false;
        }
        RenderUtils::SetDebugName(this->uav, DebugCategory::GBUFFER, "GBuffer", name, "UAV");
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
    SafeRelease(this->uav);
}