#include "deferred_pass.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"

#include "assets/asset_manager.hpp"

#include <d3d11.h>

bool DeferredPass::Create(ID3D11Device *device, uint32_t width, uint32_t height)
{
	this->width = width;
	this->height = height;

#pragma region GBuffers
	{
		gBuffers.resize(+GBufferType::SIZE);

		for (size_t bufferIndex = 0; bufferIndex < +GBufferType::SIZE; ++bufferIndex)
		{
			if (!this->gBuffers[bufferIndex].Initialize(device, width, height))
			{
				LogError("Failed to create GBuffer with index %zu", bufferIndex);
			}
			if (bufferIndex == 0)
			{
				this->gBuffers[0].GetTexture()->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("Example Texture"), "Example Texture");
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

bool DeferredPass::CreateInputLayout(ID3D11Device *device)
{
	LogInfo("Creating Input Layout");
	LogIndent();

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV",		  0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT result = device->CreateInputLayout(
		layoutDesc, 4, 
		this->vertexShader->GetByteCode().data(), 
		this->vertexShader->GetByteCode().length(),
		&this->inputLayout
	);

	if (FAILED(result)) 
	{
		LogError("Failed to create Input Layout");
		return false;
	}

	LogIndent();

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

	this->vertexShader = assetManager->Get<VertexShader>("assets/vs_deferred.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/ps_deferred.cso");
	this->computeShader = assetManager->Get<ComputeShader>("assets/cs_deferred.cso");

#pragma endregion

	if (!this->Create(device, width, height))
	{
		LogError("Failed to create the Deferred Pass");
		return false;
	}

	if (!this->CreateInputLayout(device))
	{
		LogError("Failed to create Input Layout");
		return false;
	}

	LogUnindent();

	return true;
}

void DeferredPass::Shutdown()
{
	LogInfo("Shutting down Deferred Pass");

	SafeRelease(this->depthStencilTexture);
	SafeRelease(this->depthStencilView);
	SafeRelease(this->inputLayout);
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

void DeferredPass::RenderGeometryPass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources *sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
	{
		LogWarn("Could not find a primary view to render to");
		return;
	}

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(this->width);
	viewport.Height = static_cast<float>(this->height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	sharedResources->BindSamplers(deviceContext);
	sharedResources->UploadPerViewData(deviceContext, *view);

	ID3D11RenderTargetView *rtvs[3] = {
		this->GetGBuffer(GBufferType::POSITION).GetRenderTargetView(),
		this->GetGBuffer(GBufferType::NORMAL).GetRenderTargetView(),
		backBuffer.GetView(),
	};

	const float clearAlbedo[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float clearNormal[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
	const float clearPosition[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	deviceContext->ClearRenderTargetView(rtvs[0], clearAlbedo);
	deviceContext->ClearRenderTargetView(rtvs[1], clearNormal);
	deviceContext->ClearRenderTargetView(rtvs[2], clearPosition);
	deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	deviceContext->OMSetRenderTargets(3, rtvs, this->depthStencilView);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(this->inputLayout);

	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

	ID3D11Buffer *perView = sharedResources->GetPerView();
	ID3D11Buffer *perObject = sharedResources->GetPerObject();
	ID3D11Buffer *perMaterial = sharedResources->GetPerMaterial();
	deviceContext->VSSetConstantBuffers(0, 1, &perView);
	deviceContext->VSSetConstantBuffers(1, 1, &perObject);
	deviceContext->PSSetConstantBuffers(2, 1, &perMaterial);

	deviceContext->RSSetViewports(1, &viewport);

	for (GeometryCommand &command : view->queue.geometryCommands) {
		PerObjectData perObjectData{};
        XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
        XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix,XMMatrixInverse(nullptr, worldMatrix));
		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources->GetPerObject(), perObjectData);

		XMFLOAT3 diffuse = {
			command.material.baseColor.x * command.tint.x,
			command.material.baseColor.y * command.tint.y,
			command.material.baseColor.z * command.tint.z
		};

		PerMaterialData perMaterialData{};
		//perMaterialData.materialDiffuse = diffuse;
		//perMaterialData.materialSpecular = material->specularColour;
		//perMaterialData.materialSpecularExponent = material->specularExponent;

		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources->GetPerMaterial(), perMaterialData);

		//ID3D11ShaderResourceView *srvs[2] = {
		//	material->diffuseTexture.Get()->shaderResourceView,
		//	material->normalTexture.Get()->shaderResourceView
		//};
		//deviceContext->PSSetShaderResources(0, 2, srvs);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &command.vertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(command.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(command.indexCount, command.startIndex, 0);
	}

	ID3D11RenderTargetView *nullRtvs[3] = {};
	deviceContext->OMSetRenderTargets(3, nullRtvs, nullptr);

	ID3D11ShaderResourceView *nullSRVs[2] = {};
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}

void DeferredPass::RenderLightingPass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources *sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	// RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	// if (!view)
	// {
	// 		LogWarn("Could not find a primary view to render to");
	// 		return;
	// }
}

void DeferredPass::Render(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources *sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	this->RenderGeometryPass(device, deviceContext, sharedResources, backBuffer, views);
	this->RenderLightingPass(device, deviceContext, sharedResources, backBuffer, views);
}