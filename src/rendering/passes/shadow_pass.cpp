#include "shadow_pass.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"

#include "assets/asset_manager.hpp"

bool ShadowPass::Create(ID3D11Device *device, uint32_t size, uint32_t sizeDirectional)
{
	this->size = size;
	this->sizeDirectional = sizeDirectional;

#pragma region Depth Stencil
	{

		HRESULT result{};

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = static_cast<UINT>(size);
		textureDesc.Height = static_cast<UINT>(size);
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

		RenderUtils::SetDebugName(this->depthStencilTexture, DebugCategory::RENDER_PASS, "Depth Stencil", "Shadow");

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

		RenderUtils::SetDebugName(this->depthStencilView, DebugCategory::RENDER_PASS, "Depth Stencil", "Shadow", "DSV");

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

		RenderUtils::SetDebugName(this->depthStencilView, DebugCategory::RENDER_PASS, "Depth Stencil", "Shadow", "SRV");

		LogInfo("Depth Stencil Shader Resource View created");
	}
#pragma endregion

	return true;
}

bool ShadowPass::CreateInputLayout(ID3D11Device *device)
{
	LogInfo("Creating Input Layout");
	LogIndent();

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV",		  0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT result = device->CreateInputLayout(
		layoutDesc, 2,
		this->vertexShader->GetByteCode().data(),
		this->vertexShader->GetByteCode().length(),
		&this->inputLayout
	);

	if (FAILED(result))
	{
		LogError("Failed to create Input Layout");
		return false;
	}

	RenderUtils::SetDebugName(this->inputLayout, DebugCategory::RENDER_PASS, "Input Layout", "Shadow");

	LogUnindent();

	return true;
}

bool ShadowPass::Initialize(
	ID3D11Device *device,
	AssetManager *assetManager,
	uint32_t size,
	uint32_t sizeDirectional
)
{
	LogInfo("Initializing Shadow Pass...");
	LogIndent();

#pragma region Load Shaders

	this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_shadow.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_shadow.cso");
	this->pixelShaderLinear = assetManager->Get<PixelShader>("assets/shaders/ps_linear_shadow.cso");

#pragma endregion

	if (!this->Create(device, size, sizeDirectional))
	{
		LogError("Failed to create the Shadow Pass");
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

void ShadowPass::Shutdown()
{
	LogInfo("Shutting down Shadow Pass");

	SafeRelease(this->depthStencilTexture);
	SafeRelease(this->depthStencilView);
	SafeRelease(this->depthStencilSRV);
	SafeRelease(this->inputLayout);
}

void ShadowPass::RenderShadows(
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	RenderView *view
)
{
	sharedResources.UploadPerViewData(deviceContext, *view);

	for (GeometryCommand &command : view->queue.geometryCommands) {
		PerObjectData perObjectData{};
		XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
		XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix, XMMatrixInverse(nullptr, worldMatrix));
		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources.GetPerObject(), perObjectData);

		deviceContext->PSSetShaderResources(0, 1, &command.albedoSRV);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &command.vertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(command.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(command.indexCount, command.startIndex, 0);
	}
}

void ShadowPass::Render(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Shadow pass");

	std::vector<int> directionalViewIndices;
	std::vector<int> spotViewIndices;
	std::vector<int> pointViewIndices;

	for (int i = 0; i < views.size(); ++i)
	{
		if (views[i].type == ViewType::SHADOW_MAP_DIRECTIONAL)
		{
			directionalViewIndices.push_back(i);
		}
		else if (views[i].type == ViewType::SHADOW_MAP_SPOT)
		{
			spotViewIndices.push_back(i);
		}
		else if (views[i].type == ViewType::SHADOW_MAP_POINT)
		{
			pointViewIndices.push_back(i);
		}
	}

	//deviceContext->ClearDepthStencilView(this->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	sharedResources.BindSamplers(deviceContext);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(this->inputLayout);

	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);

	ID3D11Buffer *perView = sharedResources.GetPerView();
	ID3D11Buffer *perObject = sharedResources.GetPerObject();
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_OBJECT, 1, &perObject);

	// Directional lights
	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(this->sizeDirectional);
	viewport.Height = static_cast<float>(this->sizeDirectional);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	deviceContext->RSSetViewports(1, &viewport);

	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

	BeginAnnotation("Directional Lights");
	int i = 0;
	for (int index : directionalViewIndices)
	{
		RenderView &view = views[index];

		ID3D11DepthStencilView *dsv = sharedResources.GetDirectionalShadowDSV(i);
		deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		deviceContext->OMSetRenderTargets(0, nullptr, dsv);

		this->RenderShadows(deviceContext, sharedResources, &view);

		i++;
	}
	EndAnnotation();

	// Other Lights
	viewport.Width = static_cast<float>(this->size);
	viewport.Height = static_cast<float>(this->size);

	deviceContext->RSSetViewports(1, &viewport);

	BeginAnnotation("Spot Lights");
	i = 0;
	for (int index : spotViewIndices)
	{
		RenderView &view = views[index];

		ID3D11DepthStencilView *dsv = sharedResources.GetSpotShadowDSV(i);
		deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		deviceContext->OMSetRenderTargets(0, nullptr, dsv);

		this->RenderShadows(deviceContext, sharedResources, &view);

		i++;
	}
	EndAnnotation();

	ID3D11ShaderResourceView *nullSRV = {};
	deviceContext->PSSetShaderResources(0, 1, &nullSRV);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	EndAnnotation();
}