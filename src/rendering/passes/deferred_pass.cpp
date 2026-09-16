#include "deferred_pass.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

#include "assets/asset_manager.hpp"

bool DeferredPass::Create(ID3D11Device *device, uint32_t width, uint32_t height)
{
#pragma region GBuffers
	{
		gBuffers.resize(static_cast<size_t>(GBufferType::SIZE));

		for (
			size_t bufferIndex = 0;
			bufferIndex < static_cast<size_t>(GBufferType::SIZE);
			++bufferIndex
			)
		{
			if (!this->gBuffers[bufferIndex].Initialize(device, width, height))
			{
				LogError("Failed to create GBuffer with index %zu", bufferIndex);
			}
		}
	}
#pragma endregion

#pragma region Depth Stencil
	{

		HRESULT result{};

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = static_cast<UINT>(width);
		textureDesc.Height = static_cast<UINT>(height);
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		result = device->CreateTexture2D(&textureDesc, nullptr, &this->depthStencilTexture);

		if (FAILED(result))
		{
			LogError("Failed to create the Depth Stencil Texture");
			return false;
		}

		LogInfo("Depth Stencil Texture created");

		result = device->CreateDepthStencilView(this->depthStencilTexture, nullptr, &this->depthStencilView);

		if (FAILED(result))
		{
			LogError("Failed to create the Depth Stencil View");
			return false;
		}

		LogInfo("Depth Stencil View created");
	}
#pragma endregion

	return true;
}

bool DeferredPass::Initialize(
	ID3D11Device *device,
	AssetManager *assetManager,
	uint32_t width,
	uint32_t height
)
{
	LogInfo("Initializing Deferred Pass...");
	LogIndent();

#pragma region Load Shaders

	this->vertexShader = assetManager->Get<VertexShader>("assets/deferred_vertex_shader.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/deferred_pixel_shader.cso");
	this->computeShader = assetManager->Get<ComputeShader>("assets/deferred_compute_shader.cso");

#pragma endregion

	if (!this->Create(device, width, height))
	{
		LogError("Failed to create the Deferred Pass");
		return false;
	}

	LogUnindent();
}

bool DeferredPass::Resize(
	ID3D11Device *device,
	uint32_t width,
	uint32_t height
)
{
	LogInfo("Resizing Deferred Pass...");
	LogIndent();

	this->gBuffers.clear();
	SafeRelease(this->depthStencilView);
	SafeRelease(this->depthStencilTexture);

	if (!this->Create(device, width, height))
	{
		LogError("Failed to create");
		return false;
	}

	LogUnindent();

	return true;
}

DeferredPass::~DeferredPass()
{
	SafeRelease(this->depthStencilTexture);
	SafeRelease(this->depthStencilView);
}
