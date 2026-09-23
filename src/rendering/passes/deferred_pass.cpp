#include "deferred_pass.hpp"
#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"

#include "assets/asset_manager.hpp"

#include <d3d11.h>
#include <array>

bool DeferredPass::Create(ID3D11Device *device, uint32_t width, uint32_t height)
{
	this->width = width;
	this->height = height;

#pragma region GBuffers
	{
		gBuffers.resize(+GBufferType::COUNT);


		DXGI_FORMAT formats[] =
		{
			DXGI_FORMAT_R8G8B8A8_UNORM,		// Albedo | Metallic
			DXGI_FORMAT_R8G8B8A8_UNORM,		// Encoded Normal | Roughness

            DXGI_FORMAT_R32G32B32A32_FLOAT	// Output
		};

		std::string names[] =
		{
			"Albedo and Metallic",			// Albedo | Metallic
			"Normal and Roughness",			// Encoded Normal | Roughness

            "Output"						// Output
		};

		for (size_t i = 0; i < +GBufferType::COUNT; ++i)
			if (!this->gBuffers[i].Initialize(device, this->width, this->height, formats[i], names[i]))
				LogError("Failed to create GBuffer with index %zu", i);
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

	RenderUtils::SetDebugName(this->inputLayout, DebugCategory::RENDER_PASS, "Input Layout", "Deferred");

	LogUnindent();

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

	this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_deferred.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_deferred.cso");
	this->computeShader = assetManager->Get<ComputeShader>("assets/shaders/cs_deferred.cso");

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

	SafeRelease(this->inputLayout);
}

bool DeferredPass::Resize(
	ID3D11Device *device,
	uint32_t width,
	uint32_t height
)
{
	LogInfo("Resizing Deferred Pass (%u, %u)...", width, height);
	LogIndent();

	this->gBuffers.clear();
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
	SharedResources &sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Geometry Pass");

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

	sharedResources.BindSamplers(deviceContext);
	sharedResources.UploadPerViewData(deviceContext, *view);

	ID3D11RenderTargetView *rtvs[2] = {
		this->GetGBuffer(GBufferType::ALBEDO_METALLIC).GetRenderTargetView(),
		this->GetGBuffer(GBufferType::NORMAL_ROUGHNESS).GetRenderTargetView()
	};

	const float clearAlbedoSpecularExponent[4] = { 0.0f, 0.0f, 0.0f, 70.0f / 1000.0f };
	const float clearNormalSpecular[4] = { 0.5f, 0.5f, 1.0f, 0.7f };

	deviceContext->ClearRenderTargetView(rtvs[0], clearAlbedoSpecularExponent);
	deviceContext->ClearRenderTargetView(rtvs[1], clearNormalSpecular);
	deviceContext->ClearDepthStencilView(backBuffer.GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	deviceContext->OMSetRenderTargets(2, rtvs, backBuffer.GetDepthStencilView());

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(this->inputLayout);

	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);

	ID3D11Buffer *perView = sharedResources.GetPerView();
	ID3D11Buffer *perObject = sharedResources.GetPerObject();
	ID3D11Buffer *perMaterial = sharedResources.GetPerMaterial();
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_OBJECT, 1, &perObject);
	deviceContext->PSSetConstantBuffers(+ConstantBufferSlot::PER_MATERIAL, 1, &perMaterial);

	deviceContext->RSSetViewports(1, &viewport);

	std::array<ID3D11ShaderResourceView *, 3> srvs;

	for (GeometryCommand &command : view->queue.geometryCommands) {
		PerObjectData perObjectData{};
        XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
        XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix,XMMatrixInverse(nullptr, worldMatrix));
		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources.GetPerObject(), perObjectData);

		XMFLOAT3 tint = {
			command.material.baseColor.x * command.tint.x,
			command.material.baseColor.y * command.tint.y,
			command.material.baseColor.z * command.tint.z
		};

		PerMaterialData perMaterialData{};
		perMaterialData.materialTint = tint;
		perMaterialData.materialSpecular = 0.8f;
		perMaterialData.materialSpecularExponent = 80.0f;

		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources.GetPerMaterial(), perMaterialData);

		srvs = {
			command.albedoSRV,
			command.normalSRV,
			command.metallicRoughnessSRV
		};
		deviceContext->PSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, &command.vertexBuffer, &stride, &offset);
		deviceContext->IASetIndexBuffer(command.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(command.indexCount, command.startIndex, 0);
	}

	ID3D11RenderTargetView *nullRtvs[2] = {};
	deviceContext->OMSetRenderTargets(2, nullRtvs, nullptr);

	ID3D11ShaderResourceView *nullSRVs[srvs.size()] = {};
	deviceContext->PSSetShaderResources(0, static_cast<UINT>(srvs.size()), nullSRVs);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	EndAnnotation();
}

void DeferredPass::RenderLightingPass(
    ID3D11Device *device,
    ID3D11DeviceContext *deviceContext,
    SharedResources &sharedResources,
	BackBuffer &backBuffer,
    std::vector<RenderView> &views
)
{
	BeginAnnotation("Light Pass");

	 RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	 if (!view)
	 {
	    LogWarn("Could not find a primary view to render to");
	    return;
	 }

     sharedResources.UploadLights(deviceContext, *view);

	 ID3D11UnorderedAccessView *outputUAV = this->GetGBuffer(GBufferType::OUTPUT).GetUnorderedAccessView();
	 deviceContext->CSSetUnorderedAccessViews(0, 1, &outputUAV, nullptr);

	 std::array<ID3D11ShaderResourceView *, 6> srvs = {
		 // GBuffers
		 this->GetGBuffer(GBufferType::ALBEDO_METALLIC).GetShaderResourceView(),
		 this->GetGBuffer(GBufferType::NORMAL_ROUGHNESS).GetShaderResourceView(),
		 backBuffer.GetDepthStencilSRV(),
		 // Light Buffer
		 sharedResources.GetDirectionalLightsSRV(),
		 sharedResources.GetSpotLightsSRV(),
		 sharedResources.GetPointLightsSRV()
	 };
	 deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());

	 ID3D11Buffer *light = sharedResources.GetLightBuffer();
	 deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::LIGHT, 1, &light);

	 deviceContext->CSSetShader(this->computeShader->GetShader(), nullptr, 0);

	 ID3D11Buffer *perView = sharedResources.GetPerView();
	 deviceContext->CSSetConstantBuffers(0, 1, &perView);

	 const UINT groupsX = (this->width + 7) / 8;
	 const UINT groupsY = (this->height + 7) / 8;
	 deviceContext->Dispatch(groupsX, groupsY, 1);

	 ID3D11ShaderResourceView *nullSrvs[srvs.size()] = {};
	 deviceContext->CSSetShaderResources(0, static_cast<UINT>(srvs.size()), nullSrvs);

	 ID3D11UnorderedAccessView *nullUav = nullptr;
	 deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

	 deviceContext->CSSetShader(nullptr, nullptr, 0);

	 EndAnnotation();
}

void DeferredPass::Render(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	BackBuffer &backBuffer,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Deferred Pass");

	this->RenderGeometryPass(device, deviceContext, sharedResources, backBuffer, views);
	this->RenderLightingPass(device, deviceContext, sharedResources, backBuffer, views);

	EndAnnotation();
}

DeferredPass::OutputTextures DeferredPass::GetOutputTextures() {
    return {
        this->GetGBuffer(GBufferType::ALBEDO_METALLIC).GetShaderResourceView(),
        this->GetGBuffer(GBufferType::NORMAL_ROUGHNESS).GetShaderResourceView(),
        this->GetGBuffer(GBufferType::OUTPUT).GetShaderResourceView()
    };
}