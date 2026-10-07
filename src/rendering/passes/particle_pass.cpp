#include "particle_pass.hpp"

#include "core/logging.hpp"
#include "rendering/render_utils.hpp"
#include "rendering/render_data.hpp"
#include "assets/asset_manager.hpp"

void ParticlePass::UpdateParticlePass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Compute Pass");

	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
	{
		LogWarn("Could not find a primary view to render to");
		return;
	}

	for (const ParticleSystemCommand &command : view->queue.particleSystemCommand)
	{
		ParticleSystemData data = {};
		data.count = command.count;
		data.lifeTime = command.lifeTime;
		data.spawnRadius = command.spawnRadius;
		data.atlasSize = command.atlasSize;
		data.velocity = command.velocity;
		data.animationSpeed = command.animationSpeed;
		data.acceleration = command.acceleration;
		data.startScale = command.startScale;
		data.startTint = command.startTint;
		data.endScale = command.endScale;
		data.endTint = command.endTint;
		data.flags = command.flags;

		// Upload Particle System CBuffer
		RenderUtils::UploadConstantBuffer(deviceContext, this->particleSystemCB, data);
		deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::PARTICLE, 1, &this->particleSystemCB);

		ID3D11Buffer *perFrame = sharedResources.GetPerFrame();
		deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::PER_FRAME, 1, &perFrame);

		PerObjectData perObjectData{};
		XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
		XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix, XMMatrixInverse(nullptr, worldMatrix));

		ID3D11Buffer *perObject = sharedResources.GetPerObject();
		RenderUtils::UploadConstantBuffer(deviceContext, perObject, perObjectData);
		deviceContext->CSSetConstantBuffers(+ConstantBufferSlot::PER_OBJECT, 1, &perObject);

		// Bind Particle SB
		deviceContext->CSSetUnorderedAccessViews(0, 1, &command.particleUAV, nullptr);

		// Bind Shader
		deviceContext->CSSetShader(this->computeShader->GetShader(), nullptr, 0);

		// Dispatch Shader
		constexpr UINT THREAD_GROUPS_COUNT = 32;
		UINT threads = static_cast<UINT>(std::ceil(command.count / (float)THREAD_GROUPS_COUNT) * THREAD_GROUPS_COUNT);
		deviceContext->Dispatch(threads, 1, 1);

		// Unbind Particle SB
		ID3D11UnorderedAccessView *nullUav = nullptr;
		deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

		// Unbind Shader
		deviceContext->CSSetShader(nullptr, nullptr, 0);
	}

	EndAnnotation();
}

void ParticlePass::RenderDrawPass(
	ID3D11Device *device,
	ID3D11DeviceContext *deviceContext,
	SharedResources &sharedResources,
	ID3D11RenderTargetView *renderTargetView,
	ID3D11DepthStencilView *depthStencilView,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Draw Pass");

	RenderView *view = RenderUtils::GetView(views, ViewType::PRIMARY);
	if (!view)
	{
		LogWarn("Could not find a primary view to render to");
		return;
	}

	sharedResources.BindSamplers(deviceContext);
	sharedResources.UploadPerViewData(deviceContext, *view);

	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	deviceContext->IASetInputLayout(nullptr);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	ID3D11Buffer *perFrame = sharedResources.GetPerFrame();
	ID3D11Buffer *perView = sharedResources.GetPerView();
	ID3D11Buffer *perObject = sharedResources.GetPerObject();
	ID3D11Buffer *perMaterial = sharedResources.GetPerMaterial();

	deviceContext->GSSetConstantBuffers(+ConstantBufferSlot::PER_VIEW, 1, &perView);

	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PER_OBJECT, 1, &perObject);
	deviceContext->VSSetConstantBuffers(+ConstantBufferSlot::PARTICLE, 1, &this->particleSystemCB);

	deviceContext->PSSetConstantBuffers(+ConstantBufferSlot::PER_FRAME, 1, &perFrame);
	deviceContext->PSSetConstantBuffers(+ConstantBufferSlot::PARTICLE, 1, &this->particleSystemCB);

	/* Shaders */
	deviceContext->VSSetShader(this->vertexShader->GetShader(), nullptr, 0);
	deviceContext->PSSetShader(this->pixelShader->GetShader(), nullptr, 0);
	deviceContext->GSSetShader(this->geometryShader->GetShader(), nullptr, 0);

	for (const ParticleSystemCommand &command : view->queue.particleSystemCommand)
	{
		PerObjectData perObjectData{};
		XMMATRIX worldMatrix = XMLoadFloat4x4(&command.worldMatrix);
		XMStoreFloat4x4(&perObjectData.worldMatrix, XMMatrixTranspose(worldMatrix));
		XMStoreFloat4x4(&perObjectData.worldInverseTransposeMatrix, XMMatrixInverse(nullptr, worldMatrix));
		RenderUtils::UploadConstantBuffer(deviceContext, sharedResources.GetPerObject(), perObjectData);

		ParticleSystemData data = {};
		data.count = command.count;
		data.lifeTime = command.lifeTime;
		data.spawnRadius = command.spawnRadius;
		data.atlasSize = command.atlasSize;
		data.velocity = command.velocity;
		data.animationSpeed = command.animationSpeed;
		data.acceleration = command.acceleration;
		data.startScale = command.startScale;
		data.startTint = command.startTint;
		data.endScale = command.endScale;
		data.endTint = command.endTint;
		data.flags = command.flags;

		// Upload Particle System CBuffer
		RenderUtils::UploadConstantBuffer(deviceContext, this->particleSystemCB, data);
		deviceContext->PSSetConstantBuffers(+ConstantBufferSlot::PARTICLE, 1, &this->particleSystemCB);

		UINT sampleMask = 0xffffffff;
		if ((command.flags & +ParticleFlags::ADDITIVE) == +ParticleFlags::ADDITIVE)
		{
			deviceContext->OMSetBlendState(this->additiveBlendState, nullptr, sampleMask);
			deviceContext->OMSetDepthStencilState(this->additiveDepthStencilState, 0);
		}
		else
		{
			deviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);
			deviceContext->OMSetDepthStencilState(nullptr, 0);
		}

		// Bind Particle Structured Buffer
		deviceContext->VSSetShaderResources(0, 1, &command.particleSRV);
		// Bind Texture
		deviceContext->PSSetShaderResources(0, 1, &command.colorSRV);
		
		deviceContext->Draw(command.count, 0);
	}

	// Unbind SRVs
	ID3D11ShaderResourceView *nullSrvs[1] = {};
	deviceContext->VSSetShaderResources(0, 1, nullSrvs);
	deviceContext->PSSetShaderResources(0, 1, nullSrvs);

	// Unbind Shaders
	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);

	// Reset
	UINT sampleMask = 0xffffffff;
	deviceContext->OMSetBlendState(nullptr, nullptr, sampleMask);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	EndAnnotation();
}

void ParticlePass::Render(
	ID3D11Device *device, 
	ID3D11DeviceContext *deviceContext, 
	SharedResources &sharedResources, 
	ID3D11RenderTargetView *renderTargetView,
	ID3D11DepthStencilView *depthStencilView,
	std::vector<RenderView> &views
)
{
	BeginAnnotation("Particle Pass");

	this->UpdateParticlePass(device, deviceContext, sharedResources, views);
	this->RenderDrawPass(device, deviceContext, sharedResources, renderTargetView, depthStencilView, views);

	EndAnnotation();
}

bool ParticlePass::Initialize(ID3D11Device* device, AssetManager *assetManager)
{
	LogInfo("Initializing Particle Pass...");
	LogIndent();

	if (!RenderUtils::CreateConstantBuffer(
		device,
		sizeof(ParticleSystemData),
		&this->particleSystemCB,
		"Particle System Data")
	)
	{
		return false;
	}
	
	if (!CreateAdditiveStates(device))
	{
		LogError("Failed to create blend states");
		return false;
	}

	this->vertexShader = assetManager->Get<VertexShader>("assets/shaders/vs_particle_system.cso");
	this->pixelShader = assetManager->Get<PixelShader>("assets/shaders/ps_particle_system.cso");
	this->geometryShader = assetManager->Get<GeometryShader>("assets/shaders/gs_particle_system.cso");
	this->computeShader = assetManager->Get<ComputeShader>("assets/shaders/cs_particle_system.cso");

	LogUnindent();

	return true;
}

bool ParticlePass::CreateAdditiveStates(ID3D11Device *device)
{
	HRESULT result;

	/* Blend State */
	D3D11_BLEND_DESC desc = {};
	desc.RenderTarget[0].BlendEnable = true;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = device->CreateBlendState(&desc, &this->additiveBlendState);

	if (FAILED(result))
	{
		LogError("DirectX failed to create Blend State");
		return false;
	}

	RenderUtils::SetDebugName(this->additiveBlendState, DebugCategory::RENDER_PASS, "", "Additive", "Blend State");

	/* Depth Stencil State */
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.StencilEnable = false;
	
	result = device->CreateDepthStencilState(&dsDesc, &this->additiveDepthStencilState);

	if (FAILED(result))
	{
		LogError("DirectX failed to create Depth Stencil State");
		return false;
	}

	RenderUtils::SetDebugName(this->additiveDepthStencilState, DebugCategory::RENDER_PASS, "", "Additive", "Depth Stencil State");

	return true;
}

void ParticlePass::Shutdown()
{
	LogInfo("Shutting down Particle Pass");

	SafeRelease(this->additiveBlendState);
	SafeRelease(this->additiveDepthStencilState);
	SafeRelease(this->particleSystemCB);
}